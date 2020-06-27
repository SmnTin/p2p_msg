#ifndef P2P_MSG_IS_BIG_ENDIAN_H
#define P2P_MSG_IS_BIG_ENDIAN_H

#include <cstdint>

namespace p2p {
    bool isBigEndian() {
        uint16_t x = 1;
        return *((uint8_t *) &x) == 0;
    }
}

#endif //P2P_MSG_IS_BIG_ENDIAN_H
