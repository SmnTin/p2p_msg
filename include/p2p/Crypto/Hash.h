#ifndef P2P_MSG_CRYPTO_HASH_H
#define P2P_MSG_CRYPTO_HASH_H

#include <string>

namespace p2p::Crypto::Hash {
    std::string SHA256(const std::string &data);

    std::string SHA256(std::string &&data);

    std::string SHA256(const void *data, int len);
}

#endif //P2P_MSG_CRYPTO_HASH_H
