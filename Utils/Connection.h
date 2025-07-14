#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

template<typename T>
struct Connection {
    long int conn;
    long int addr;
    std::string name;
};

#endif