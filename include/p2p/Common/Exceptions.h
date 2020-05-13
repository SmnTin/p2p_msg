#ifndef P2P_MSG_COMMON_EXCEPTIONS_H
#define P2P_MSG_COMMON_EXCEPTIONS_H

namespace p2p {
    class NotImplementedException : public std::logic_error {
    public:
        NotImplementedException()
                : logic_error("Not implemented.") {}
    };
}

#endif //P2P_MSG_COMMON_EXCEPTIONS_H
