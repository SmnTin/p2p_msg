#ifndef P2P_MSG_NONCECHECKERTEST_H
#define P2P_MSG_NONCECHECKERTEST_H

#include "gtest/gtest.h"
#include "p2p/Basic/Network/NonceChecker.h"

using namespace p2p::Basic::Network;

namespace {
    TEST(NonceChecker, TestNewNonce) {
        NonceChecker<uint8_t, 5> _checker;
        ASSERT_TRUE(_checker.checkAndUpdate(100));
        ASSERT_TRUE(_checker.checkAndUpdate(101));
        ASSERT_TRUE(_checker.checkAndUpdate(200));
    }

    TEST(NonceChecker, GeneralTest) {
        NonceChecker<uint8_t, 5> _checker;
        ASSERT_TRUE(_checker.checkAndUpdate(100));
        ASSERT_TRUE(_checker.checkAndUpdate(98));
        ASSERT_TRUE(_checker.checkAndUpdate(97));
        ASSERT_TRUE(_checker.checkAndUpdate(104));
        ASSERT_FALSE(_checker.checkAndUpdate(100));
        ASSERT_FALSE(_checker.checkAndUpdate(99));
    }

}

#endif //P2P_MSG_NONCECHECKERTEST_H
