#ifndef CRYPTO_H
#define CRYPTO_H

#include <inttypes.h>
#include <string>
#include <algorithm>
#include <openssl/aes.h>
#include <openssl/rand.h>

class Crypto {
    public: 
    std::array<unsigned char, 512> encryptData(unsigned char* DATA, AES_KEY* KEY);
    
};


#endif