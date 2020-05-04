#include "gtest/gtest.h"
#include "p2p/Crypto/CryptoTest.h"
#include "p2p/Common/diTest.h"
#include "p2p/Basic/Network/StreamTest.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}