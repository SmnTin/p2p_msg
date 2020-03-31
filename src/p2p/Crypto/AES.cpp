#include "p2p/Crypto/AES.h"
#include "AES/AES.h"
#include "cstring"

namespace p2p::Crypto::AES {

    uint8_t *encryptECB(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen) {
        class AES aes(256);
        unsigned int _outLen;
        uint8_t _key[32];
        memcpy(_key, key.data(), 32);
        auto result = aes.EncryptECB((uint8_t *)in, len, _key, _outLen);
        outLen = _outLen;
        return result;
    }
    uint8_t *decryptECB(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen) {
        class AES aes(256);
        uint8_t _key[32];
        memcpy(_key, key.data(), 32);
        auto result = aes.DecryptECB((uint8_t *)in, len, _key);
        outLen = len;
        return result;
    }
}