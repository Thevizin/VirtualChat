#ifndef MESSAGESBLOCK_H
#define MESSAGESBLOCK_H

#include "Message.h"
#include <string>

const int MAX_MESSAGES = 100;

class MessagesBlock {
public:
    using StringMessage = Message<std::string>;

private:
    struct Queue {
        int front, back;
        StringMessage queue[MAX_MESSAGES];

        void start();
        bool full() const;
        bool empty() const;
        void enqueue(const StringMessage& x);
        void dequeue();
        StringMessage get_last() const;
        int size() const;
        StringMessage get_at(int index) const;
    };

    Queue messageQueue;

public:
    void addMessage(const StringMessage& msg);
    StringMessage getMessageFrom(const std::string& env) const;
    StringMessage getHistoryMessage(int index) const;
    Queue& getQueue();
    const Queue& getQueue() const;
};

#endif
