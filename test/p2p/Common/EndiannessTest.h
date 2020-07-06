#ifndef P2P_MSG_ENDIANNESSTEST_H
#define P2P_MSG_ENDIANNESSTEST_H

#include "gtest/gtest.h"
#include "p2p/Common/Endianness.h"

using namespace p2p;

namespace {
    TEST(Endianness, convertSimpleTypes) {
        uint16_t host16 = 40200;
        uint16_t network16 = hostToNetworkByteOrder(host16);
        uint16_t newHost16 = networkToHostByteOrder(network16);
        ASSERT_EQ(host16, newHost16);

        uint32_t host32 = 1.8e9 + 123545;
        uint32_t network32 = hostToNetworkByteOrder(host32);
        uint32_t newHost32 = networkToHostByteOrder(network32);
        ASSERT_EQ(host32, newHost32);

        uint64_t host64 = 1.8e16 + 3463463;
        uint64_t network64 = hostToNetworkByteOrder(host64);
        uint64_t newHost64 = networkToHostByteOrder(network64);
        ASSERT_EQ(host64, newHost64);
    }

    TEST(Endianness, convertComplexTypes) {
        struct F {
            uint32_t a;
            uint16_t b;
            bool operator==(const F& f) const {
                return a == f.a && b == f.b;
            }
        };

        srand(time(0));
        F host {(uint32_t)rand(), (uint16_t)rand()};
        F network = hostToNetworkByteOrder(host);
        F newHost = networkToHostByteOrder(network);

        ASSERT_EQ(host, newHost);
    }
}

#endif //P2P_MSG_ENDIANNESSTEST_H
