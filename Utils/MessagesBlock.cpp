#include "MessagesBlock.h"
#include <stdexcept>

// ========== Queue Methods ==========

void MessagesBlock::Queue::start() {
    front = -1;
    back = -1;
}

bool MessagesBlock::Queue::full() const {
    return ((back + 1) % MAX_MESSAGES == front);
}

bool MessagesBlock::Queue::empty() const {
    return (front == -1);
}

void MessagesBlock::Queue::enqueue(const StringMessage& x) {
    if (full()) dequeue();

    if (empty()) {
        front = back = 0;
    } else {
        back = (back + 1) % MAX_MESSAGES;
    }
    queue[back] = x;
}

void MessagesBlock::Queue::dequeue() {
    if (empty()) return;

    if (front == back) {
        front = back = -1;
    } else {
        front = (front + 1) % MAX_MESSAGES;
    }
}

MessagesBlock::StringMessage MessagesBlock::Queue::get_last() const {
    if (empty()) throw std::runtime_error("Queue is empty");
    return queue[back];
}

int MessagesBlock::Queue::size() const {
    if (empty()) return 0;
    if (back >= front) return back - front + 1;
    return MAX_MESSAGES - front + back + 1;
}

MessagesBlock::StringMessage MessagesBlock::Queue::get_at(int index) const {
    if (empty() || index < 0 || index >= size())
        throw std::out_of_range("Invalid index");
    return queue[(front + index) % MAX_MESSAGES];
}

// ========== MessagesBlock Methods ==========

void MessagesBlock::addMessage(const StringMessage& msg) {
    messageQueue.enqueue(msg);
}

MessagesBlock::StringMessage MessagesBlock::getMessageFrom(const std::string& env) const {
    int sz = messageQueue.size();
    for (int i = sz - 1; i >= 0; --i) {
        StringMessage m = messageQueue.get_at(i);
        if (m.envoy == env) return m;
    }
    throw std::runtime_error("No message from specified sender.");
}

MessagesBlock::StringMessage MessagesBlock::getHistoryMessage(int index) const {
    return messageQueue.get_at(index);
}

MessagesBlock::Queue& MessagesBlock::getQueue() {
    return messageQueue;
}

const MessagesBlock::Queue& MessagesBlock::getQueue() const {
    return messageQueue;
}
