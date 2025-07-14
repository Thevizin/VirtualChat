#ifndef CONNECTIONBLOCK_H
#define CONNECTIONBLOCK_H

#include "Connection.h"
#include <string>
#include <vector>

const int HASH_TABLE_SIZE = 101;

class ConnectionsBlock {
public:
    using StringConnection = Connection<std::string>; // Definindo o tipo aqui

private:
    struct HashNode {
        StringConnection connection;
        HashNode* next;
        
        HashNode(const StringConnection& conn) : connection(conn), next(nullptr) {}
    };

    HashNode* hashTable[HASH_TABLE_SIZE];
    
    int hashFunction(int conn) const;
    int hashFunction(const std::string& name) const;

public:
    ConnectionsBlock();
    ~ConnectionsBlock();
    
    void addConnection(const StringConnection& conn);
    void removeConnection(int conn);
    int get_addr(int conn) const;
    std::string get_name(int conn) const;
    std::string get_names() const;
    StringConnection getConnection(const std::string& name) const;
    
    ConnectionsBlock(const ConnectionsBlock&) = delete;
    ConnectionsBlock& operator=(const ConnectionsBlock&) = delete;
};

#endif