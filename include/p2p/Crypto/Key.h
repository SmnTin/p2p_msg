#ifndef P2P_MSG_CRYPTO_KEY_H
#define P2P_MSG_CRYPTO_KEY_H

#include <array>

namespace p2p::Crypto {
    template<std::size_t SIZE>
    using Key = std::array<uint8_t, SIZE>;
}

#endif //P2P_MSG_CRYPTO_KEY_H
