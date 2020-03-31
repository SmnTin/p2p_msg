#include "gtest/gtest.h"
#include "p2p/Crypto/CryptoTest.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}