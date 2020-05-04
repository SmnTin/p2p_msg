#ifndef P2P_MSG_DITEST_H
#define P2P_MSG_DITEST_H

#include "gtest/gtest.h"
#include "p2p/Common/di.h"

using namespace p2p;

namespace {
    struct A {
        A() {
            static int id = 0;
            _id = ++id;
        }

        int _id;
    };
    struct B {
        B(std::shared_ptr<A> a) : _a(a) {

        }
        std::shared_ptr<A> _a;
    };

    TEST(DI, MultipleCreation) {
        auto inj = di::make_injector();
        auto a = inj.create<std::shared_ptr<A>>();
        auto b = inj.create<std::shared_ptr<B>>();
        ASSERT_EQ(a->_id, b->_a->_id);
    }
}

#endif //P2P_MSG_DITEST_H
