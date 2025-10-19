#include "ServerBlock.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <memory> // make_unique

using namespace std;
#define BUFFER_SIZE 1024

// ------------------------------------------------------------
// Construtor / Destrutor
// ------------------------------------------------------------
ServerBlock::ServerBlock(bool isPrivate)
    : isPrivate(isPrivate)
{
    cout << "Servidor criado (" << (isPrivate ? "privado" : "publico") << ")." << endl;
}

ServerBlock::~ServerBlock() {
    begin_deleting();
}

// ------------------------------------------------------------
// Controle de inicialização / encerramento
// ------------------------------------------------------------
void ServerBlock::exitServer() {
    lock_guard<mutex> lock(connections_mutex);
    for (auto &c : clients) {
        closesocket(c.socket);
    }
    clients.clear();
    // destruir grupos
    privateGroups.clear(); // unique_ptr garante destruição correta
}

void ServerBlock::begin_deleting() {
    exitServer();
}

// ------------------------------------------------------------
// Utilitários de cliente
// ------------------------------------------------------------
void ServerBlock::send_message_to_user(ClientConnection& client, const string& msg) {
    send(client.socket, msg.c_str(), (int)msg.size(), 0);
}

ServerBlock::ClientConnection* ServerBlock::find_client_by_name(const string& name) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto &c : clients)
        if (c.name == name)
            return &c;
    return nullptr;
}

ServerBlock::ClientConnection* ServerBlock::find_client_by_socket(SOCKET socket) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto &c : clients)
        if (c.socket == socket)
            return &c;
    return nullptr;
}

string ServerBlock::list_connected_users() {
    lock_guard<mutex> lock(connections_mutex);
    stringstream ss;
    ss << "Usuarios conectados: ";
    bool first = true;
    for (const auto &c : clients) {
        if (!first) ss << ", ";
        ss << c.name;
        first = false;
    }
    ss << "\n";
    return ss.str();
}

string ServerBlock::list_users() {
    return list_connected_users();
}

// ------------------------------------------------------------
// Grupos (agora com unique_ptr)
// ------------------------------------------------------------
void ServerBlock::create_private_group(const string& group_name) {
    lock_guard<mutex> lock(connections_mutex);
    // verifica duplicado
    for (const auto &gptr : privateGroups) {
        if (gptr && gptr->get_name() == group_name) {
            cout << "Tentativa de criar grupo ja existente: " << group_name << endl;
            return;
        }
    }
    privateGroups.emplace_back(std::make_unique<Group>(group_name));
    cout << "Grupo privado criado: " << group_name << endl;
}

Group* ServerBlock::find_group_by_name(const string& group_name) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto &gptr : privateGroups) {
        if (gptr && gptr->get_name() == group_name)
            return gptr.get();
    }
    return nullptr;
}

// ------------------------------------------------------------
// Clientes
// ------------------------------------------------------------
void ServerBlock::add_client(SOCKET client_socket, const std::string& name) {
    lock_guard<mutex> lock(connections_mutex);
    ClientConnection client;
    client.socket = client_socket;
    client.name = name;
    clients.push_back(client);

    ConnectionsBlock::StringConnection conn = {
        (long int)client_socket,
        0,
        name
    };
    
    connectionsBlock.addConnection(conn);

    cout << "Novo cliente no servidor publico: " << name << endl;

    // Notifica outros usuários
    for (auto &c : clients)
        if (c.socket != client_socket)
            send_message_to_user(c, "[SISTEMA] " + name + " entrou no chat.\n");
}

ServerBlock::ClientConnection* ServerBlock::get_last_client() {
    lock_guard<mutex> lock(connections_mutex);
    if (clients.empty()) return nullptr;
    return &clients.back();
}

void ServerBlock::set_name(const string server_name) {
    name = isPrivate ? server_name : "MainServer";
}

string ServerBlock::get_name() {
    return name;
}

// ------------------------------------------------------------
// Função principal de tratamento do cliente
// ------------------------------------------------------------
void ServerBlock::handle_client(ClientConnection& client) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    string welcome = "Bem-vindo, " + client.name + "!\n"
                     "Comandos:\n"
                     "- mensagem normal: global ou grupo\n"
                     "- /list: listar usuarios\n"
                     "- /creategroup [nome]\n"
                     "- /changegroup [nome]\n"
                     "- /leave (se estiver num grupo)\n"
                     "- /private [nome] [mensagem]\n\n";
    send_message_to_user(client, welcome);

    while ((bytes_read = recv(client.socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        string input(buffer);
        input.erase(remove(input.begin(), input.end(), '\n'), input.end());
        input.erase(remove(input.begin(), input.end(), '\r'), input.end());

        if (input.empty()) continue;

        // ---------------- /list ----------------
        if (input == "/list") {
            if (client.currentGroup)
                send_message_to_user(client, client.currentGroup->list_connected_users());
            else
                send_message_to_user(client, list_users());
            continue;
        }

        // ---------------- /creategroup ----------------
        if (input.rfind("/creategroup", 0) == 0) {
            size_t space = input.find(' ');
            if (space == string::npos) {
                send_message_to_user(client, "Uso: /creategroup [nome]\n");
                continue;
            }
            string group_name = input.substr(space + 1);
            if (group_name.empty()) {
                send_message_to_user(client, "Nome de grupo invalido.\n");
                continue;
            }

            if (find_group_by_name(group_name)) {
                send_message_to_user(client, "Grupo ja existe.\n");
                continue;
            }

            create_private_group(group_name);
            Group* newGroup = find_group_by_name(group_name);
            if (newGroup) {
                newGroup->add_client(client.socket, client.name);
                client.currentGroup = newGroup;
                send_message_to_user(client, "Grupo criado e voce entrou em " + group_name + ".\n");
            }
            continue;
        }

        // ---------------- /changegroup ----------------
        if (input.rfind("/changegroup", 0) == 0) {
            size_t space = input.find(' ');
            if (space == string::npos) {
                send_message_to_user(client, "Uso: /changegroup [nome]\n");
                continue;
            }
            string group_name = input.substr(space + 1);

            if (group_name == "General" || group_name == "global") {
                if (client.currentGroup) {
                    client.currentGroup->remove_client(client.socket);
                    client.currentGroup = nullptr;
                    send_message_to_user(client, "Voce voltou ao chat global.\n");
                } else {
                    send_message_to_user(client, "Ja esta no chat global.\n");
                }
                continue;
            }

            Group* grp = find_group_by_name(group_name);
            if (!grp) {
                send_message_to_user(client, "Grupo nao encontrado.\n");
                continue;
            }

            if (client.currentGroup)
                client.currentGroup->remove_client(client.socket);

            grp->add_client(client.socket, client.name);
            client.currentGroup = grp;
            send_message_to_user(client, "Voce entrou no grupo " + group_name + ".\n");
            continue;
        }

        // ---------------- /leave ----------------
        if (input == "/leave") {
            if (client.currentGroup) {
                string gname = client.currentGroup->get_name();

                // Apenas remove o cliente do grupo, sem fechar socket
                client.currentGroup->remove_client(client.socket);
                client.currentGroup = nullptr;

                send_message_to_user(client, "Você saiu do grupo " + gname + " e voltou ao chat global.\n");
                cout << client.name << " voltou ao chat global." << endl;
            } else {
                send_message_to_user(client, "Você não está em nenhum grupo.\n");
            }
            continue;
        }

        // ---------------- /adduser ----------------
        if (input.rfind("/adduser", 0) == 0) {
            size_t space = input.find(' ');
            if (space == string::npos) {
                send_message_to_user(client, "Uso: /adduser [nome]\n");
                continue;
            }
            string username = input.substr(space + 1);
            if (username.empty()) {
                send_message_to_user(client, "Uso: /adduser [nome]\n");
                continue;
            }

            if (!client.currentGroup) {
                send_message_to_user(client, "Você precisa estar em um grupo para adicionar membros.\n");
                continue;
            }

            ClientConnection* target = find_client_by_name(username);
            if (!target) {
                send_message_to_user(client, "Usuário '" + username + "' não encontrado no global.\n");
                continue;
            }

            // Verifica se já está no mesmo grupo
            Group* grp = client.currentGroup;
            string group_name = grp->get_name();

            // Adiciona ao grupo atual
            grp->add_client(target->socket, target->name);
            target->currentGroup = grp;

            send_message_to_user(client, "Você adicionou " + username + " ao grupo " + group_name + ".\n");
            send_message_to_user(*target, "[SISTEMA] Você foi adicionado ao grupo " + group_name + " por " + client.name + ".\n");

            cout << client.name << " adicionou " << username << " ao grupo " << group_name << endl;
            continue;
        }
        // ---------------- /private ----------------
        if (input.rfind("/private", 0) == 0) {
            size_t space1 = input.find(' ');
            size_t space2 = input.find(' ', space1 + 1);
            if (space1 == string::npos || space2 == string::npos) {
                send_message_to_user(client, "Formato: /private [nome] [mensagem]\n");
                continue;
            }
            string target_name = input.substr(space1 + 1, space2 - space1 - 1);
            string content = input.substr(space2 + 1);

            ClientConnection* target = find_client_by_name(target_name);
            if (!target) {
                send_message_to_user(client, "Usuario '" + target_name + "' nao encontrado.\n");
                continue;
            }

            Message<string> msg;
            msg.envoy = client.name;
            msg.content = content;
            msg.msg_type = "msg";
            msg.destination = target_name;

            target->privateHistory[client.name].addMessage(msg);
            client.privateHistory[target_name].addMessage(msg);

            send_message_to_user(*target, "[PRIVADA de " + client.name + "]: " + content + "\n");
            send_message_to_user(client, "[ENVIADA para " + target_name + "]: " + content + "\n");
            continue;
        }

        // ---------------- Mensagem normal ----------------
        if (client.currentGroup) {
            string msg = "[GRUPO " + client.currentGroup->get_name() + "] " + client.name + ": " + input + "\n";
            client.currentGroup->send_message(client.socket, msg);
        } else {
            string msg = "[GLOBAL] " + client.name + ": " + input + "\n";
            broadcast_message(client.socket, msg);
        }
    }

    // Se cliente desconectou
    if (client.currentGroup)
        client.currentGroup->remove_client(client.socket);

    remove_client(client.socket);
}

// ------------------------------------------------------------
// Remoção e broadcast
// ------------------------------------------------------------
void ServerBlock::remove_client(SOCKET client_socket) {
    lock_guard<mutex> lock(connections_mutex);
    auto it = find_if(clients.begin(), clients.end(),
                      [&](const ClientConnection &c) { return c.socket == client_socket; });
    if (it == clients.end()) return;

    string name = it->name;
    closesocket(it->socket);
    clients.erase(it);
    connectionsBlock.removeConnection((int)client_socket);

    for (auto &c : clients)
        send_message_to_user(c, "[SISTEMA] " + name + " saiu do chat.\n");

    cout << "Cliente desconectado: " << name << endl;
}

void ServerBlock::broadcast_message(SOCKET sender_socket, const string& msg) {
    lock_guard<mutex> lock(connections_mutex);
    for (auto &c : clients)
        if (c.socket != sender_socket)
            send_message_to_user(c, msg);
}

string ServerBlock::get_user_name(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    string prompt = "Digite seu nome de usuario: ";
    send(client_socket, prompt.c_str(), (int)prompt.size(), 0);

    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) return "";

    buffer[bytes_read] = '\0';
    string name(buffer);
    name.erase(remove(name.begin(), name.end(), '\n'), name.end());
    name.erase(remove(name.begin(), name.end(), '\r'), name.end());

    // Verifica duplicado
    {
        lock_guard<mutex> lock(connections_mutex);
        for (auto &c : clients)
            if (c.name == name) {
                string err = "Nome ja em uso.\n";
                send(client_socket, err.c_str(), (int)err.size(), 0);
                return "";
            }
    }
    return name;
}
