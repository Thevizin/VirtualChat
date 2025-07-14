#include "ConnectionsBlock.h"
#include <sstream>

// Definindo o tipo para uso no .cpp
using StringConnection = ConnectionsBlock::StringConnection;

ConnectionsBlock::ConnectionsBlock() {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        hashTable[i] = nullptr;
    }
}

ConnectionsBlock::~ConnectionsBlock() {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashNode* current = hashTable[i];
        while (current != nullptr) {
            HashNode* next = current->next;
            delete current;
            current = next;
        }
    }
}

int ConnectionsBlock::hashFunction(int conn) const {
    return conn % HASH_TABLE_SIZE;
}

int ConnectionsBlock::hashFunction(const std::string& name) const {
    int hash = 0;
    for (char c : name) {
        hash = (hash * 31 + c) % HASH_TABLE_SIZE;
    }
    return hash;
}

void ConnectionsBlock::addConnection(const StringConnection& conn) {
    int index = hashFunction(conn.conn);
    
    HashNode* current = hashTable[index];
    while (current != nullptr) {
        if (current->connection.conn == conn.conn) {
            current->connection = conn;
            return;
        }
        current = current->next;
    }
    
    HashNode* newNode = new HashNode(conn);
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

void ConnectionsBlock::removeConnection(int conn) {
    int index = hashFunction(conn);
    HashNode* current = hashTable[index];
    HashNode* prev = nullptr;
    
    while (current != nullptr) {
        if (current->connection.conn == conn) {
            if (prev == nullptr) {
                hashTable[index] = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            return;
        }
        prev = current;
        current = current->next;
    }
}

int ConnectionsBlock::get_addr(int conn) const {
    int index = hashFunction(conn);
    HashNode* current = hashTable[index];
    
    while (current != nullptr) {
        if (current->connection.conn == conn) {
            return current->connection.addr;
        }
        current = current->next;
    }
    
    return -1;
}

std::string ConnectionsBlock::get_name(int conn) const {
    int index = hashFunction(conn);
    HashNode* current = hashTable[index];
    
    while (current != nullptr) {
        if (current->connection.conn == conn) {
            return current->connection.name;
        }
        current = current->next;
    }
    
    return "";
}

std::string ConnectionsBlock::get_names() const {
    std::stringstream ss;
    bool first = true;
    
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashNode* current = hashTable[i];
        while (current != nullptr) {
            if (!first) {
                ss << ", ";
            }
            ss << current->connection.name;
            first = false;
            current = current->next;
        }
    }
    
    return ss.str();
}

ConnectionsBlock::StringConnection ConnectionsBlock::getConnection(const std::string& name) const {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashNode* current = hashTable[i];
        while (current != nullptr) {
            if (current->connection.name == name) {
                return current->connection;
            }
            current = current->next;
        }
    }
    
    return StringConnection{-1, -1, ""};
}