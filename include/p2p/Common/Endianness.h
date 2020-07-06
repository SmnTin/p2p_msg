#ifndef P2P_MSG_ENDIANNESS_H
#define P2P_MSG_ENDIANNESS_H

#include "p2p/Common/IntToType.h"

#include <cstdint>
#include <netinet/in.h>

namespace p2p {
    inline bool isBigEndian() {
        uint16_t x = 1;
        return *((uint8_t *) &x) == 0;
    }

    namespace details {

        template<size_t byte, size_t size>
        inline void _reverseByteOrder(uint8_t *in, uint8_t *out, IntToType<true>) {}

        template<size_t byte, size_t size>
        inline void _reverseByteOrder(uint8_t *in, uint8_t *out, IntToType<false>) {
            out[size - byte - 1] = in[byte];
            _reverseByteOrder<byte + 1, size>(in, out, IntToType<byte + 1 == size>());
        }

        template<typename T>
        inline T reverseByteOrder(T value) {
            uint8_t out[sizeof(value)];
            uint8_t *in = reinterpret_cast<uint8_t *>(&value);
            _reverseByteOrder<0, sizeof(T)>(in, out, IntToType<false>());
            return *reinterpret_cast<T *>(out);
        }
    }

    template<typename T>
    inline T hostToNetworkByteOrder(T value) {
        if (!isBigEndian())
            return details::reverseByteOrder(value);
        else
            return value;
    }

    template<>
    inline uint8_t hostToNetworkByteOrder<uint8_t>(uint8_t value) {
        return value;
    }

    template<>
    inline uint16_t hostToNetworkByteOrder<uint16_t>(uint16_t value) {
        return htons(value);
    }

    template<>
    inline uint32_t hostToNetworkByteOrder<uint32_t>(uint32_t value) {
        return htonl(value);
    }

    template<>
    inline uint64_t hostToNetworkByteOrder<uint64_t>(uint64_t value) {
        return htonll(value);
    }

    template<typename T>
    inline T networkToHostByteOrder(T value) {
        //it's actually the same ;)
        //just reversing bytes if needed
        return hostToNetworkByteOrder(value);
    }
}

#endif //P2P_MSG_ENDIANNESS_H
