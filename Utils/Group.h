#ifndef GROUP_H
#define GROUP_H

#include "Connection.h"
#include "MessagesBlock.h"
#include "ConnectionsBlock.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
#include <mutex> // para std::mutex

class ConnectionsBlock;
class Group {
    private:
        struct ClientConnection {
            SOCKET socket;
            std::string name;
            std::unordered_map<std::string, MessagesBlock> privateHistory;
        };

        std::string name;
        MessagesBlock messages;
        ConnectionsBlock connectionsBlock;
        std::vector<ClientConnection> clients;
        std::mutex clients_mutex; // protege acesso a clients

    public:
        explicit Group(const std::string& group_name);
        ~Group();

        void add_client(SOCKET client_socket, const std::string& name);
        void remove_client(SOCKET client_socket);
        std::string list_connected_users();
        std::vector<ClientConnection> get_users();
        std::string get_name();
        void set_name(const std::string& group_name);
        // envia mensagem de um usuário para todos no grupo (exceto o remetente)
        void send_message(SOCKET sender_socket, const std::string& msg);

        // Impede cópia
        Group(const Group&) = delete;
        Group& operator=(const Group&) = delete;
};

#endif
