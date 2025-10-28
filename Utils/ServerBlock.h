#ifndef SERVERBLOCK_H
#define SERVERBLOCK_H

#include "Connection.h"
#include "MessagesBlock.h"
#include "ConnectionsBlock.h"
#include "Group.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
#include <mutex>
#include <memory> // std::unique_ptr

class ConnectionsBlock;

class ServerBlock {
private:
    // agora armazenamos ponteiros exclusivos para evitar exigir move/copy de Group
    std::vector<std::unique_ptr<Group>> privateGroups; // grupos privados criados

public:
    struct ClientConnection {
        SOCKET socket;
        std::string name;
        std::unordered_map<std::string, MessagesBlock> privateHistory;
        Group* currentGroup = nullptr; // grupo em que está (nullptr = global)
    };

    using StringConnection = Connection<std::string>;
    explicit ServerBlock(bool isPrivate = false);
    ~ServerBlock();

    void exitServer();
    void begin_deleting();

    void create_private_group(const std::string& group_name);
    Group* find_group_by_name(const std::string& name);

    ClientConnection* get_last_client();
    void send_message_to_user(ClientConnection& client, const std::string& msg);
    void send_message_to_group(ClientConnection& client, const std::string& msg);
    ClientConnection* find_client_by_name(const std::string& name);
    ClientConnection* find_client_by_socket(SOCKET socket);
    std::string list_connected_users();
    std::string list_users(); // alias
    std::string get_user_name(SOCKET client_socket);
    void add_client(SOCKET client_socket, const std::string& name);
    void handle_client(ClientConnection& client);
    void remove_client(SOCKET client_socket);
    void broadcast_message(SOCKET sender_socket, const std::string& msg);
    void group_message(ClientConnection &sender_socket, const std::string& msg);
    std::string get_name();
    void set_name(const std::string server_name);

    // Impede cópia
    ServerBlock(const ServerBlock&) = delete;
    ServerBlock& operator=(const ServerBlock&) = delete;

private:
    bool isPrivate;
    MessagesBlock messages;
    ConnectionsBlock connectionsBlock;
    std::vector<ClientConnection> clients;
    std::mutex connections_mutex;
    std::string name;
};

#endif
