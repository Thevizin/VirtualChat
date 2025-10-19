#include "Group.h"
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

#define BUFFER_SIZE 1024

// Construtor / Destrutor
Group::Group(const std::string& group_name)
    : name(group_name)
{
    cout << "Grupo criado: " << name << endl;
}

Group::~Group() {
    // Fecha sockets dos clientes (se quiser forçar desconexão ao deletar o grupo)
    lock_guard<mutex> lock(clients_mutex);
    for (auto &c : clients) {
        closesocket(c.socket);
    }
    clients.clear();
}

// Adiciona cliente no grupo
void Group::add_client(SOCKET client_socket, const std::string& client_name) {
    lock_guard<mutex> lock(clients_mutex);

    // verifica duplicado
    for (const auto &c : clients) {
        if (c.name == client_name) {
            string err = "[SISTEMA] Nome já em uso neste grupo.\n";
            send(client_socket, err.c_str(), (int)err.size(), 0);
            return;
        }
    }

    ClientConnection conn;
    conn.socket = client_socket;
    conn.name = client_name;
    clients.push_back(conn);

    // Adicionar à estrutura de conexões do grupo (se ConnectionsBlock usar esse formato)
    using StringConnection = Connection<std::string>;
    StringConnection sconn = {
        (long int)client_socket,
        0,
        client_name
    };
    connectionsBlock.addConnection(sconn);

    // Mensagem de boas-vindas
    string welcome = "Bem-vindo ao grupo " + name + ", " + client_name + "!\n";
    welcome += "Comandos do grupo:\n";
    welcome += "- mensagem normal: será enviada para todos no grupo\n";
    welcome += "- /list: listar usuarios do grupo\n";
    welcome += "- /leave: sair do grupo\n";
    send(client_socket, welcome.c_str(), (int)welcome.size(), 0);

    // Notifica outros
    for (auto &c : clients) {
        if (c.socket != client_socket) {
            string notice = "[SISTEMA] " + client_name + " entrou no grupo " + name + ".\n";
            send(c.socket, notice.c_str(), (int)notice.size(), 0);
        }
    }

    cout << "Usuario " << client_name << " adicionado ao grupo " << name << endl;
}

// Remove cliente do grupo
void Group::remove_client(SOCKET client_socket) {
    lock_guard<mutex> lock(clients_mutex);

    auto it = find_if(clients.begin(), clients.end(),
                      [&](const ClientConnection &c) { return c.socket == client_socket; });
    if (it == clients.end()) return;

    string name_removed = it->name;
    // não fechar socket — usuário pode voltar ao global

    // remove da lista de conexões do grupo
    connectionsBlock.removeConnection((int)client_socket);

    clients.erase(it);

    // Notifica restante do grupo
    for (auto &c : clients) {
        string notice = "[SISTEMA] " + name_removed + " saiu do grupo " + name + ".\n";
        send(c.socket, notice.c_str(), (int)notice.size(), 0);
    }

    cout << "Usuario " << name_removed << " removido do grupo " << name << endl;
}

std::string Group::list_connected_users() {
    lock_guard<mutex> lock(clients_mutex);
    stringstream ss;
    ss << "Usuarios no grupo " << name << ": ";
    bool first = true;
    for (const auto &c : clients) {
        if (!first) ss << ", ";
        ss << c.name;
        first = false;
    }
    ss << "\n";
    return ss.str();
}

std::string Group::get_name() {
    return name;
}

void Group::set_name(const std::string& group_name) {
    name = group_name;
}

// Envia mensagem para todos no grupo, exceto o remetente (se sender_socket == INVALID_SOCKET, envia para todos)
void Group::send_message(SOCKET sender_socket, const std::string& msg) {
    lock_guard<mutex> lock(clients_mutex);

    for (auto &c : clients) {
        if (sender_socket != INVALID_SOCKET && c.socket == sender_socket) continue;
        // envia diretamente
        send(c.socket, msg.c_str(), (int)msg.size(), 0);
    }
}

