#ifndef P2P_MSG_HOST_HOSTID_H
#define P2P_MSG_HOST_HOSTID_H

namespace p2p {
    class IHostId {
    public:
        virtual NodeId getId() = 0;
    };
    typedef std::shared_ptr<IHostId> IHostIdPtr;
}

#endif //P2P_MSG_HOSTID_H
