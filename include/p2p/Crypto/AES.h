#ifndef P2P_MSG_AES_H
#define P2P_MSG_AES_H

#include "p2p/Crypto/Key.h"

namespace p2p::Crypto::AES {
    typedef Key<32> SharedKey;

    uint8_t *encryptECB(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen);
    uint8_t *decryptECB(void *in, std::size_t len, const SharedKey &key, std::size_t &outLen);
}

#endif //P2P_MSG_AES_H
