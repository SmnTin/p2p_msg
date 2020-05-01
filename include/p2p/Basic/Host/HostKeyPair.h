#ifndef P2P_MSG_HOSTID_H
#define P2P_MSG_HOSTID_H

#include "p2p/Host/HostId.h"
#include "p2p/Crypto/ECC.h"

namespace p2p::Basic {
    class HostKeyPair : public HostId {
    public:
        HostKeyPair() {
            _keys = ECC::generateKeys();
            _id = ECC::NodeIdFactory::fromPublicKey(_keys.publicKey);
        }

        ECC::KeyPair getKeys() {
            return _keys;
        }

        NodeId getId() override {
            return _id;
        }

    private:
        ECC::KeyPair _keys;
        NodeId _id;
    };
}

#endif //P2P_MSG_HOSTID_H
