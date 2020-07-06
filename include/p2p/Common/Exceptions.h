#ifndef P2P_MSG_COMMON_EXCEPTIONS_H
#define P2P_MSG_COMMON_EXCEPTIONS_H

#include "p2p/p2pException.h"
#include "p2p/p2pAssert.h"

namespace p2p {
    class NotImplementedException : public std::logic_error {
    public:
        NotImplementedException()
                : logic_error("Not implemented.") {}
    };
}

#endif //P2P_MSG_COMMON_EXCEPTIONS_H
