#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

template<typename T>
struct Message {
    int size;                // timestamp ou tamanho
    int type;                // numérico para controle
    std::string destination; // destinatário ("all", user, group_id)
    bool group = false;    // é mensagem de grupo?
    std::string envoy;       // remetente
    T content;               // conteúdo da mensagem     
    std::string msg_type;    // "msg" ou "file"
    std::string filename;    // se for arquivo
};

#endif
