#include <iostream>
#include <winsock2.h>
#include <memory>
#include <thread>
#include <chrono>
#include <ws2tcpip.h>
#include <unordered_map>
#include "ServerBlock.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080

using namespace std;

bool init_winsock() {
    WSADATA wsaData;
    return (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
}

void cleanup_winsock() {
    WSACleanup();
}

std::unordered_map<std::string, std::thread> client_threads;

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

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "Erro no bind." << endl;
        closesocket(server_fd);
        cleanup_winsock();
        return 1;
    }

    listen(server_fd, 5);
    cout << "Servidor principal rodando na porta " << PORT << endl;

    // Cria o servidor principal
    ServerBlock mainServer(false); // false = servidor público
    vector<unique_ptr<ServerBlock>> privateServers;

    // Thread que aceita novas conexões
    thread thread_accept([&]() {
        while (true) {
            sockaddr_in client_addr{};
            int addrlen = sizeof(client_addr);
            SOCKET new_socket = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
            if (new_socket == INVALID_SOCKET) continue;

            string user = mainServer.get_user_name(new_socket);
            if (user.empty()) {
                closesocket(new_socket);
                continue;
            }

            mainServer.add_client(new_socket, user);
            cout << "Novo cliente conectado: " << user << endl;

            // Cria thread dedicada ao cliente
            client_threads[user] = thread([&mainServer, new_socket]() {
                ServerBlock::ClientConnection* client = mainServer.find_client_by_socket(new_socket);
                if (!client) {
                    cerr << "Erro: cliente nao encontrado apos add_client (socket: " << new_socket << ")\n";
                    return;
                }

                try {
                    mainServer.handle_client(*client);
                } catch (const std::exception& e) {
                    cerr << "Excecao em handle_client: " << e.what() << endl;
                }

                cout << "Thread encerrada para cliente (socket: " << new_socket << ")\n";
            });

            client_threads[user].detach();
        }
    });

    thread_accept.join();

    closesocket(server_fd);
    cleanup_winsock();
    return 0;
}

