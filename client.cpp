#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <sstream>
#include <vector>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool running = true;
bool in_group = false;
string current_group = "General"; // sala padrão
vector<string> grupos;

// Recebe mensagens do servidor
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
    if (!in_group) {
        cout << "\n========== COMANDOS DISPONÍVEIS: MENU PADRÃO ==========\n";
        cout << "- Digite normalmente: mensagem para todos OU grupo atual\n";
        cout << "- /list: listar usuários conectados\n";
        cout << "- /groups: listar grupos que você participa\n";
        cout << "- /create_private [nome_sala]: cria sala privada\n";
        cout << "- /join [sala]: entra em uma sala existente\n";
        cout << "- /use [sala]: muda grupo ativo para enviar mensagens\n";
        cout << "- /help: mostrar esta ajuda\n";
        cout << "- /quit: sair do chat\n";
        cout << "=======================================================\n";
    } else {
        cout << "\n========== COMANDOS DISPONÍVEIS: MENU GRUPO ==========\n";
        cout << "- Digite normalmente: mensagem para todos do grupo\n";
        cout << "- /list: listar usuários conectados no grupo\n";
        cout << "- /add [user]: adiciona usuário ao grupo\n";
        cout << "- /remove [user]: remove usuário do grupo\n";
        cout << "- /leave: sai do grupo atual\n";
        cout << "- /use General: volta para o chat público\n";
        cout << "- /help: mostrar esta ajuda\n";
        cout << "- /quit: sair do chat\n";
        cout << "======================================================\n";
    }
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

        if (input.empty()) continue;

        // ----------- COMANDOS LOCAIS -----------
        if (input == "/help") {
            show_help();
            continue;
        }

        if (input == "/quit") {
            running = false;
            break;
        }

        if (input == "/groups") {
            cout << "\n=== Grupos que você participa ===\n";
            if (grupos.empty()) cout << "Nenhum grupo privado.\n";
            else for (auto &g : grupos) cout << "- " << g << endl;
            cout << "=================================\n";
            continue;
        }

        if (input.rfind("/use ", 0) == 0) {
            string sala = input.substr(5);
            if (sala == "General") {
                in_group = false;
                current_group = "General";
                cout << "Você voltou para o chat público.\n";
            } else if (find(grupos.begin(), grupos.end(), sala) != grupos.end()) {
                in_group = true;
                current_group = sala;
                cout << "Você agora está no grupo '" << sala << "'.\n";
            } else {
                cout << "Você não participa do grupo '" << sala << "'.\n";
            }
            continue;
        }

        if (input.rfind("/leave", 0) == 0) {
            if (!in_group) {
                cout << "Você não está em um grupo.\n";
                continue;
            }
            cout << "Você saiu do grupo '" << current_group << "'.\n";
            grupos.erase(remove(grupos.begin(), grupos.end(), current_group), grupos.end());
            in_group = false;
            current_group = "General";
            continue;
        }

        // ----------- COMANDOS ENVIADOS AO SERVIDOR -----------
        if (input.rfind("/create_private ", 0) == 0) {
            string sala = input.substr(16);
            if (!sala.empty()) {
                grupos.push_back(sala);
                cout << "Grupo privado '" << sala << "' criado e adicionado à sua lista.\n";
            }
        }

        else if (input.rfind("/join ", 0) == 0) {
            string sala = input.substr(6);
            if (!sala.empty()) {
                if (find(grupos.begin(), grupos.end(), sala) == grupos.end()) {
                    grupos.push_back(sala);
                    cout << "Você entrou no grupo '" << sala << "'.\n";
                } else {
                    cout << "Você já participa do grupo '" << sala << "'.\n";
                }
            }
        }

        // envia o comando original (para o servidor lidar também)
        send(sock, input.c_str(), (int)input.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
