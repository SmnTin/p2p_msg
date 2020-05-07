#include "p2p/Crypto/Hash.h"
#include "hash-lib/sha256.h"

namespace p2p::Crypto::Hash {
    std::string SHA256(const std::string &data) {
        class SHA256 sha256;
        return sha256(data);
    }

    std::string SHA256(std::string &&data) {
        class SHA256 sha256;
        return sha256(data);
    }

    std::string SHA256(const void *data, int len) {
        class SHA256 sha256;
        return sha256(data, len);
    }
}