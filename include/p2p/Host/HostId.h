#ifndef P2P_MSG_HOST_HOSTID_H
#define P2P_MSG_HOST_HOSTID_H

namespace p2p {
    class HostId {
    public:
        virtual NodeId getId() = 0;
    };
}

#endif //P2P_MSG_HOSTID_H
