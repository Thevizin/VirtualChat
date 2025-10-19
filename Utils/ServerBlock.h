#ifndef SERVERBLOCK_H
#define SERVERBLOCK_H

#include "Connection.h"
#include "MessagesBlock.h"
#include "ConnectionsBlock.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
#include <mutex> // para std::mutex


// Declaração antecipada — evita dependências circulares
class ConnectionsBlock;

class ServerBlock {
private:
    // Estrutura representando um cliente conectado
    struct ClientConnection {
        SOCKET socket;
        std::string name;
        std::unordered_map<std::string, MessagesBlock> privateHistory;
    };

public:
    using StringConnection = Connection<std::string>;

    explicit ServerBlock(bool isPrivate = false);
    ~ServerBlock();

    void exitServer();
    void begin_deleting();

    void send_message_to_user(ClientConnection& client, const std::string& msg);
    ClientConnection* find_client_by_name(const std::string& name);
    ClientConnection* find_client_by_socket(SOCKET socket);
    std::string list_connected_users();
    std::string get_user_name(SOCKET client_socket);
    void add_client(SOCKET client_socket, const std::string& name);
    void remove_client(SOCKET client_socket);
    void broadcast_message(SOCKET sender_socket, const std::string& msg);
    std::string list_users(); // alias de list_connected_users()

    // Impede cópia
    ServerBlock(const ServerBlock&) = delete;
    ServerBlock& operator=(const ServerBlock&) = delete;

private:
    bool isPrivate;
    MessagesBlock messages;
    ConnectionsBlock connectionsBlock;
    std::vector<ClientConnection> clients;
    std::mutex connections_mutex;
};

#endif
