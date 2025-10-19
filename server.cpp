#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Utils/ServerBlock.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

// Mapa de servidores (público + privados)
unordered_map<string, ServerBlock*> serverRooms;
mutex serverRooms_mutex;

// Inicializa o Winsock
bool init_winsock() {
    WSADATA wsaData;
    return (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
}

// Finaliza o Winsock
void cleanup_winsock() {
    WSACleanup();
}

// Busca (ou cria) sala
ServerBlock* get_or_create_room(const string& name, bool isPrivate = false) {
    lock_guard<mutex> lock(serverRooms_mutex);
    auto it = serverRooms.find(name);
    if (it != serverRooms.end())
        return it->second;

    ServerBlock* newRoom = new ServerBlock(isPrivate);
    serverRooms[name] = newRoom;
    cout << "Nova sala criada: " << name << " (" << (isPrivate ? "privada" : "pública") << ")" << endl;
    return newRoom;
}

// 👂 Thread de cliente
void handle_client(SOCKET client_socket, sockaddr_in client_addr) {
    // nome do usuário obtido no ServerBlock (por login, nickname etc)
    string user_name = get_or_create_room("publico")->get_user_name(client_socket);
    if (user_name.empty()) {
        closesocket(client_socket);
        return;
    }

    cout << "Novo cliente: " << user_name << endl;
    ServerBlock* currentRoom = get_or_create_room("publico");
    string currentRoomName = "publico";

    {
        lock_guard<mutex> lock(serverRooms_mutex);
        currentRoom->add_client(client_socket, user_name);
    }

    char buffer[BUFFER_SIZE];

    while (true) {
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) break;

        buffer[bytes_read] = '\0';
        string input(buffer);

        // limpa \n\r
        input.erase(remove(input.begin(), input.end(), '\n'), input.end());
        input.erase(remove(input.begin(), input.end(), '\r'), input.end());

        // -------------------- COMANDOS --------------------

        if (input.rfind("/create_private", 0) == 0) {
            string room_name = input.substr(15);
            if (room_name.empty()) {
                string msg = "Uso: /create_private nome_sala\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
                continue;
            }

            {
                lock_guard<mutex> lock(serverRooms_mutex);
                if (serverRooms.count(room_name)) {
                    string msg = "A sala '" + room_name + "' já existe.\n";
                    send(client_socket, msg.c_str(), (int)msg.size(), 0);
                    continue;
                }

                ServerBlock* newRoom = new ServerBlock(true);
                serverRooms[room_name] = newRoom;
                newRoom->add_client(client_socket, user_name);
            }

            string msg = "Sala privada '" + room_name + "' criada e você foi adicionado.\n";
            send(client_socket, msg.c_str(), (int)msg.size(), 0);
            continue;
        }

        else if (input.rfind("/join", 0) == 0) {
            string room_name = input.substr(6);
            if (room_name.empty()) {
                string msg = "Uso: /join nome_sala\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
                continue;
            }

            lock_guard<mutex> lock(serverRooms_mutex);
            auto it = serverRooms.find(room_name);
            if (it == serverRooms.end()) {
                string msg = "Sala '" + room_name + "' não existe.\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
            } else {
                ServerBlock* targetRoom = it->second;
                targetRoom->add_client(client_socket, user_name);
                string msg = "Você entrou na sala '" + room_name + "'. Use /use " + room_name + " para ativar.\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
            }
            continue;
        }

        else if (input.rfind("/use", 0) == 0) {
            string room_name = input.substr(5);
            lock_guard<mutex> lock(serverRooms_mutex);

            if (room_name == "General" || room_name == "publico") {
                currentRoom = serverRooms["publico"];
                currentRoomName = "publico";
                string msg = "Você voltou para o chat público.\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
            } else if (serverRooms.count(room_name)) {
                currentRoom = serverRooms[room_name];
                currentRoomName = room_name;
                string msg = "Agora você está usando o grupo '" + room_name + "'.\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
            } else {
                string msg = "A sala '" + room_name + "' não existe.\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
            }
            continue;
        }

        else if (input == "/leave") {
            if (currentRoomName == "publico") {
                string msg = "Você não pode sair do chat público.\n";
                send(client_socket, msg.c_str(), (int)msg.size(), 0);
                continue;
            }

            lock_guard<mutex> lock(serverRooms_mutex);
            currentRoom->remove_client(client_socket);

            currentRoom = serverRooms["publico"];
            currentRoomName = "publico";
            string msg = "Você saiu do grupo e voltou ao chat público.\n";
            send(client_socket, msg.c_str(), (int)msg.size(), 0);
            continue;
        }

        else if (input == "/list") {
            string userList = currentRoom->list_users();
            send(client_socket, userList.c_str(), (int)userList.size(), 0);
            continue;
        }

        // -------------------- MENSAGENS NORMAIS --------------------

        currentRoom->broadcast_message(
            client_socket,
            "[" + currentRoomName + "] " + user_name + ": " + input + "\n"
        );
    }

    // desconectou
    lock_guard<mutex> lock(serverRooms_mutex);
    for (auto& [name, room] : serverRooms)
        room->remove_client(client_socket);

    closesocket(client_socket);
    cout << user_name << " desconectou-se." << endl;
}

// -------------------- MAIN --------------------
int main() {
    if (!init_winsock()) {
        cerr << "Erro ao inicializar Winsock." << endl;
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        cerr << "Erro ao criar socket." << endl;
        cleanup_winsock();
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cerr << "Erro no bind." << endl;
        closesocket(server_fd);
        cleanup_winsock();
        return 1;
    }

    if (listen(server_fd, 3) == SOCKET_ERROR) {
        cerr << "Erro no listen." << endl;
        closesocket(server_fd);
        cleanup_winsock();
        return 1;
    }

    cout << "Servidor principal rodando na porta " << PORT << endl;

    // Cria sala pública principal
    get_or_create_room("publico", false);

    while (true) {
        sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        SOCKET new_socket = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
        if (new_socket == INVALID_SOCKET) continue;

        thread(handle_client, new_socket, client_addr).detach();
    }

    closesocket(server_fd);
    cleanup_winsock();
    return 0;
}
