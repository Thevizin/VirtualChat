#include <iostream>
#include "Utils/MessagesBlock.h"

using namespace std;

MessagesBlock GlobalMessages[MAX_MESSAGES];

void handle_messages(MessagesBlock& block) {
    // Exemplo: pegar todas as mensagens e imprimir
    int total = block.getQueue().size();
    for (int i = 0; i < total; ++i) {
        auto msg = block.getHistoryMessage(i);
        cout << "[" << i << "] " << msg.envoy << " -> " << msg.destination
             << ": " << msg.content << endl;
    }
}

int main() {
    cout << "Iniciando...\n";

    MessagesBlock& bloco = GlobalMessages[0];
    bloco.getQueue().start();

    // Adiciona mensagens
    bloco.addMessage({10, 1, "DestinoA", "Vinicius", "Olá!"});
    bloco.addMessage({15, 1, "DestinoB", "Lucas", "Oi!"});
    bloco.addMessage({20, 1, "DestinoA", "Vinicius", "Como vai?"});

    // Pega a última mensagem de "Vinicius"
    try {
        auto ultima = bloco.getMessageFrom("Vinicius");
        cout << "\nÚltima mensagem de Vinicius: " << ultima.content << endl;
    } catch (exception& e) {
        cout << "Erro: " << e.what() << endl;
    }

    // Exibe histórico completo
    cout << "\nHistórico completo:\n";
    handle_messages(bloco);

    return 0;
}
