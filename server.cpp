#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Utils/MessagesBlock.h"
#include "Utils/Message.h"
#include "Utils/ConnectionsBlock.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

// Estrutura para conexões de clientes
struct ClientConnection {
    SOCKET socket;
    string name;
    // Histórico de mensagens privadas deste cliente
    unordered_map<string, MessagesBlock> privateHistory;
};

// Lista de conexões
vector<ClientConnection> connections;
mutex connections_mutex;

// Buffer de mensagens globais
MessagesBlock GlobalMessages;

// Bloco de conexões para busca por nome
ConnectionsBlock connectionsBlock;

// Inicializa o Winsock
bool init_winsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    return (result == 0);
}

// Finaliza o Winsock
void cleanup_winsock() {
    WSACleanup();
}

// Envia mensagem para um cliente específico
void send_message_to_user(ClientConnection &client, const string &msg) {
    send(client.socket, msg.c_str(), (int)msg.size(), 0);
}

// Função para parsear comando de mensagem
struct ParsedMessage {
    string type;        // "all", "private"
    string destination; // nome do usuário (para private)
    string content;     // conteúdo da mensagem
};

ParsedMessage parse_message(const string &input) {
    ParsedMessage parsed;
    
    if (input.substr(0, 8) == "/private") {
        parsed.type = "private";
        size_t space1 = input.find(' ', 8);
        size_t space2 = input.find(' ', space1 + 1);
        
        if (space1 != string::npos && space2 != string::npos) {
            parsed.destination = input.substr(space1 + 1, space2 - space1 - 1);
            parsed.content = input.substr(space2 + 1);
        } else {
            parsed.type = "error";
            parsed.content = "Formato incorreto. Use: /private [nome] [mensagem]";
        }
    } else if (input == "/list") {
        parsed.type = "list";
    } else {
        parsed.type = "all";
        parsed.content = input;
    }
    
    return parsed;
}

// Encontra cliente por nome
ClientConnection* find_client_by_name(const string &name) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto &conn : connections) {
        if (conn.name == name) {
            return &conn;
        }
    }
    return nullptr;
}

// Encontra cliente por socket
ClientConnection* find_client_by_socket(SOCKET socket) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto &conn : connections) {
        if (conn.socket == socket) {
            return &conn;
        }
    }
    return nullptr;
}

// Lista usuários conectados
string list_connected_users() {
    lock_guard<mutex> lock(connections_mutex);
    stringstream ss;
    ss << "Usuários conectados: ";
    bool first = true;
    for (const auto &conn : connections) {
        if (!first) ss << ", ";
        ss << conn.name;
        first = false;
    }
    return ss.str();
}

// Thread para cada cliente
void handle_client(ClientConnection client) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Envia mensagem de boas-vindas
    string welcome_msg = "Bem-vindo, " + client.name + "!\n";
    welcome_msg += "Comandos disponíveis:\n";
    welcome_msg += "- Digite normalmente para enviar mensagem para todos\n";
    welcome_msg += "- /private [nome] [mensagem] para enviar mensagem privada\n";
    welcome_msg += "- /list para ver usuários conectados\n";
    send_message_to_user(client, welcome_msg);

    while ((bytes_read = recv(client.socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        string input(buffer);
        
        // Remove quebras de linha
        input.erase(remove(input.begin(), input.end(), '\n'), input.end());
        input.erase(remove(input.begin(), input.end(), '\r'), input.end());

        ParsedMessage parsed = parse_message(input);
        
        if (parsed.type == "list") {
            string user_list = list_connected_users();
            send_message_to_user(client, user_list);
            continue;
        }
        
        if (parsed.type == "error") {
            send_message_to_user(client, parsed.content);
            continue;
        }

        // Cria a mensagem
        Message<string> msg;
        msg.envoy = client.name;
        msg.content = parsed.content;
        msg.msg_type = "msg";
        msg.destination = parsed.destination;

        if (parsed.type == "all") {
            msg.destination = "all";
            GlobalMessages.addMessage(msg);

            // Broadcast para todos os outros clientes
            lock_guard<mutex> lock(connections_mutex);
            for (auto &conn : connections) {
                if (conn.socket != client.socket) {
                    send_message_to_user(conn, "[GLOBAL] " + client.name + ": " + msg.content);
                }
            }
        }
        else if (parsed.type == "private") {
            // Busca o destinatário
            ClientConnection* target = find_client_by_name(parsed.destination);
            
            if (target == nullptr) {
                send_message_to_user(client, "Usuário '" + parsed.destination + "' não encontrado.");
                continue;
            }

            // Adiciona mensagem ao histórico do remetente
            ClientConnection* sender = find_client_by_socket(client.socket);
            if (sender) {
                sender->privateHistory[parsed.destination].addMessage(msg);
            }

            // Adiciona mensagem ao histórico do destinatário
            target->privateHistory[client.name].addMessage(msg);

            // Envia mensagem para o destinatário
            send_message_to_user(*target, "[PRIVADA de " + client.name + "]: " + msg.content);
            
            // Confirma para o remetente
            send_message_to_user(client, "[ENVIADA para " + parsed.destination + "]: " + msg.content);
        }
    }

    // Cliente desconectou
    closesocket(client.socket);

    // Remove das conexões
    {
        lock_guard<mutex> lock(connections_mutex);
        connections.erase(
            remove_if(connections.begin(), connections.end(),
                      [&](const ClientConnection &c) { return c.socket == client.socket; }),
            connections.end());
    }

    // Remove do bloco de conexões
    ConnectionsBlock::StringConnection conn_to_remove = {(long int)client.socket, 0, client.name};
    connectionsBlock.removeConnection((int)client.socket);

    cout << "Cliente desconectado: " << client.name << endl;

    // Notifica outros usuários
    {
        lock_guard<mutex> lock(connections_mutex);
        for (auto &conn : connections) {
            send_message_to_user(conn, "[SISTEMA] " + client.name + " desconectou-se.");
        }
    }
}

// Função para definir nome do usuário
string get_user_name(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    string name_prompt = "Digite seu nome de usuário: ";
    send(client_socket, name_prompt.c_str(), (int)name_prompt.size(), 0);

    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        string name(buffer);
        
        // Remove quebras de linha
        name.erase(remove(name.begin(), name.end(), '\n'), name.end());
        name.erase(remove(name.begin(), name.end(), '\r'), name.end());
        
        // Verifica se o nome já existe
        {
            lock_guard<mutex> lock(connections_mutex);
            for (const auto &conn : connections) {
                if (conn.name == name) {
                    string error_msg = "Nome já em uso. Conexão encerrada.\n";
                    send(client_socket, error_msg.c_str(), (int)error_msg.size(), 0);
                    return "";
                }
            }
        }
        
        return name;
    }
    
    return "";
}

// Função principal do servidor
int main() {
    if (!init_winsock()) {
        cerr << "Erro ao inicializar Winsock" << endl;
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        cerr << "Erro ao criar socket" << endl;
        cleanup_winsock();
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        cerr << "Erro no bind" << endl;
        closesocket(server_fd);
        cleanup_winsock();
        return 1;
    }

    if (listen(server_fd, 3) == SOCKET_ERROR) {
        cerr << "Erro no listen" << endl;
        closesocket(server_fd);
        cleanup_winsock();
        return 1;
    }

    cout << "Servidor rodando na porta " << PORT << endl;
    cout << "Aguardando conexões..." << endl;

    while (true) {
        sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        SOCKET new_socket = accept(server_fd, (sockaddr *)&client_addr, &addrlen);
        if (new_socket == INVALID_SOCKET) {
            cerr << "Erro no accept" << endl;
            continue;
        }

        // Solicita nome do usuário
        string user_name = get_user_name(new_socket);
        if (user_name.empty()) {
            closesocket(new_socket);
            continue;
        }

        // Cria um cliente
        ClientConnection client;
        client.socket = new_socket;
        client.name = user_name;

        // Adiciona às conexões
        {
            lock_guard<mutex> lock(connections_mutex);
            connections.push_back(client);
        }

        // Adiciona ao bloco de conexões
        ConnectionsBlock::StringConnection conn = {
            (long int)new_socket, 
            (long int)client_addr.sin_addr.s_addr, 
            user_name
        };
        connectionsBlock.addConnection(conn);

        cout << "Novo cliente conectado: " << user_name << endl;

        // Notifica outros usuários
        {
            lock_guard<mutex> lock(connections_mutex);
            for (auto &conn : connections) {
                if (conn.socket != new_socket) {
                    send_message_to_user(conn, "[SISTEMA] " + user_name + " entrou no chat.");
                }
            }
        }

        thread(handle_client, client).detach();
    }

    closesocket(server_fd);
    cleanup_winsock();
    return 0;
}