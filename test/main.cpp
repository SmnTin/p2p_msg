#include "gtest/gtest.h"

#include "p2p/Crypto/CryptoTest.h"
#include "p2p/Common/diTest.h"
#include "p2p/Basic/Network/StreamTest.h"
#include "p2p/Network/AnySelectorTest.h"
#include "p2p/Basic/Routers/ProtocolRouterTest.h"
#include "p2p/Basic/Transports/UDPTest.h"
#include "p2p/Common/EndiannessTest.h"
#include "p2p/Basic/Network/NonceCheckerTest.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}