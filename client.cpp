#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool running = true;

void receive_messages(SOCKET sock) {
    char buffer[1024];
    int bytes_received;
    while (running) {
        bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            cout << "\n" << buffer << "\n> " << flush;
        } else if (bytes_received == 0) {
            cout << "\nConexão encerrada pelo servidor.\n";
            running = false;
            break;
        }
    }
}

void show_help() {
    cout << "\n========== CHAT VIRTUAL ==========\n";
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Erro ao conectar ao servidor.\n";
        return 1;
    }

    cout << "Conectado ao servidor!\n";
    
    thread(receive_messages, sock).detach();
    
    show_help();

    string input;
    while (running) {
        cout << "> ";
        getline(cin, input);
        
        if (input == "/help") {
            show_help();
            continue;
        }
        
        if (input == "/quit") {
            running = false;
            break;
        }
        
        if (input.empty()) {
            continue;
        }

        send(sock, input.c_str(), input.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}