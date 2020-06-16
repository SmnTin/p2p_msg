#ifndef P2P_MSG_INTTOTYPE_H
#define P2P_MSG_INTTOTYPE_H

namespace p2p {
    template<int v>
    struct IntToType {
        enum {
            value = v;
        };
    };
}

#endif //P2P_MSG_INTTOTYPE_H
