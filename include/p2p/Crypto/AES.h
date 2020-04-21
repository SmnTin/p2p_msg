#ifndef P2P_MSG_AES_H
#define P2P_MSG_AES_H

#include "p2p/Crypto/Key.h"

#include <memory>
#include <future>
#include <thread>
#include <string>

namespace p2p::Crypto::AES {
    typedef Key<32> SharedKey;

    uint8_t *encryptECB(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen);
    uint8_t *decryptECB(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen);
    std::unique_ptr<uint8_t[]> encryptECBSafe(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen);
    std::unique_ptr<uint8_t[]> decryptECBSafe(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen);
    std::string encryptECB(const std::string &in, const SharedKey &key);
    std::string decryptECB(const std::string &in, const SharedKey &key);
}

#endif //P2P_MSG_AES_H
