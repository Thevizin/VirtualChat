#ifndef MESSAGESBLOCK_H
#define MESSAGESBLOCK_H

#include <string>

const int MAX_MESSAGES = 100;

class MessagesBlock {
    struct Message {
        int size;
        int type;
        std::string destination;
        std::string envoy;
        std::string content;
    };

    struct Queue {
        int front, back;
        Message queue[MAX_MESSAGES];

        void start();
        bool full();
        bool empty();
        void enqueue(Message x);
        void dequeue();
        Message get_last();
        int size();
        Message get_at(int index);
    };

    Queue messageQueue;

public:
    Message getMessageFrom(std::string env);
    void addMessage(Message msg);
    Message getHistoryMessage(int index);

    // Dá acesso externo para quem quiser manipular diretamente a fila
    Queue& getQueue() { return messageQueue; }
};

#endif
