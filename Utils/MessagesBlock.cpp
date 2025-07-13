#include "MessagesBlock.h"
#include <iostream>
#include <stdexcept>

using namespace std;

// ==================== MÉTODOS DE Queue ====================
void MessagesBlock::Queue::start() {
    front = -1;
    back = -1;
}

bool MessagesBlock::Queue::full() {
    return ((back + 1) % MAX_MESSAGES == front);
}

bool MessagesBlock::Queue::empty() {
    return (front == -1);
}

void MessagesBlock::Queue::enqueue(Message x) {
    if (full()) dequeue();

    if (empty()) {
        front = back = 0;
    } else {
        back = (back + 1) % MAX_MESSAGES;
    }
    queue[back] = x;
}

void MessagesBlock::Queue::dequeue() {
    if (empty()) {
        cout << "The queue is empty" << endl;
    } else if (front == back) {
        front = -1;
        back = -1;
    } else {
        front = (front + 1) % MAX_MESSAGES;
    }
}

MessagesBlock::Message MessagesBlock::Queue::get_last() {
    if (empty()) {
        throw runtime_error("Queue is empty, cannot retrieve last message.");
    }
    return queue[back];
}

int MessagesBlock::Queue::size() {
    if (empty()) return 0;
    if (back >= front) return back - front + 1;
    return MAX_MESSAGES - front + back + 1;
}

MessagesBlock::Message MessagesBlock::Queue::get_at(int index) {
    if (empty() || index < 0 || index >= size()) {
        throw out_of_range("Invalid index");
    }
    return queue[(front + index) % MAX_MESSAGES];
}

// ==================== MÉTODOS DE MessagesBlock ====================
MessagesBlock::Message MessagesBlock::getMessageFrom(string env) {
    int sz = messageQueue.size();

    for (int i = sz - 1; i >= 0; i--) {
        Message m = messageQueue.get_at(i);
        if (m.envoy == env) {
            return m;
        }
    }
    throw runtime_error("No message found from the specified sender.");
}

void MessagesBlock::addMessage(Message msg) {
    messageQueue.enqueue(msg);
}

MessagesBlock::Message MessagesBlock::getHistoryMessage(int index) {
    return messageQueue.get_at(index);
}
