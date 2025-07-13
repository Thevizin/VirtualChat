#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

template<typename T>
struct Message {
    int size;
    int type;
    std::string destination;
    std::string envoy;
    T content;
};

#endif
