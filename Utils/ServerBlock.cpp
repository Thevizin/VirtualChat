#include "ServerBlock.h"
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;

// Construtor
ServerBlock::ServerBlock(bool isPrivate) : isPrivate(isPrivate) {
    // Inicializa blocos e estruturas
    cout << "Servidor criado (" << (isPrivate ? "privado" : "público") << ")." << endl;
}

// Destrutor
ServerBlock::~ServerBlock() {
    cout << "Encerrando servidor..." << endl;
    begin_deleting();
}

// Métodos principais
void ServerBlock::exitServer() {
    cout << "Servidor encerrado manualmente." << endl;
}

void ServerBlock::begin_deleting() {
    lock_guard<mutex> lock(connections_mutex);
    for (auto& c : clients) {
        closesocket(c.socket);
    }
    clients.clear();
}

// Envia mensagem a um cliente
void ServerBlock::send_message_to_user(ClientConnection& client, const string& msg) {
    send(client.socket, msg.c_str(), static_cast<int>(msg.size()), 0);
}

void ServerBlock::add_client(SOCKET client_socket, const std::string& name) {
    lock_guard<mutex> lock(connections_mutex);
    ClientConnection newClient{client_socket, name, {}};
    clients.push_back(newClient);

    std::string msg = name + " entrou na sala.\n";
    for (auto& c : clients) {
        if (c.socket != client_socket)
            send(c.socket, msg.c_str(), static_cast<int>(msg.size()), 0);
    }
}

void ServerBlock::remove_client(SOCKET client_socket) {
    lock_guard<mutex> lock(connections_mutex);
    clients.erase(
        std::remove_if(clients.begin(), clients.end(),
            [client_socket](const ClientConnection& c) {
                return c.socket == client_socket;
            }),
        clients.end()
    );
}

void ServerBlock::broadcast_message(SOCKET sender_socket, const std::string& msg) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto& client : clients) {
        if (client.socket != sender_socket) {
            send(client.socket, msg.c_str(), static_cast<int>(msg.size()), 0);
        }
    }
}

std::string ServerBlock::list_users() {
    return list_connected_users();
}


// Lista usuários conectados
string ServerBlock::list_connected_users() {
    lock_guard<mutex> lock(connections_mutex);
    stringstream ss;
    ss << "Usuários conectados: ";
    bool first = true;
    for (const auto& conn : clients) {
        if (!first) ss << ", ";
        ss << conn.name;
        first = false;
    }
    return ss.str();
}

// Retorna o nome do usuário associado ao socket
string ServerBlock::get_user_name(SOCKET client_socket) {
    char buffer[1024];
    string name_prompt = "Digite seu nome de usuário: ";
    send(client_socket, name_prompt.c_str(), static_cast<int>(name_prompt.size()), 0);

    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) return "";

    buffer[bytes_read] = '\0';
    string name(buffer);

    // Remove quebras de linha
    name.erase(remove(name.begin(), name.end(), '\n'), name.end());
    name.erase(remove(name.begin(), name.end(), '\r'), name.end());

    // Verifica se o nome já existe
    lock_guard<mutex> lock(connections_mutex);
    for (const auto& conn : clients) {
        if (conn.name == name) {
            string error_msg = "Nome já em uso. Conexão encerrada.\n";
            send(client_socket, error_msg.c_str(), static_cast<int>(error_msg.size()), 0);
            return "";
        }
    }

    return name;
}

// Busca cliente pelo socket
ServerBlock::ClientConnection* ServerBlock::find_client_by_socket(SOCKET socket) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto& conn : clients) {
        if (conn.socket == socket)
            return &conn;
    }
    return nullptr;
}

// Busca cliente pelo nome
ServerBlock::ClientConnection* ServerBlock::find_client_by_name(const string& name) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto& conn : clients) {
        if (conn.name == name)
            return &conn;
    }
    return nullptr;
}

