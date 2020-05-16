#ifndef P2P_P2PASSERT_H
#define P2P_P2PASSERT_H

#include <string>

//#include "errors.h"
#include "p2p/p2pException.h"

namespace p2p {
#define p2pAssert(expr, details) \
        static_cast<bool>(expr) ? \
            void(0) \
            : \
            throw_p2p_exception((((std::string)"Assertion failed: ") + #expr + " " + details).c_str());
}

#endif
