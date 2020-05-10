#ifndef P2P_MSG_HOSTID_H
#define P2P_MSG_HOSTID_H

#include "p2p/Host/HostId.h"
#include "p2p/Crypto/ECC.h"

namespace p2p::Basic {
    class HostKeyPair : public IHostId {
    public:
        HostKeyPair() {
            _keys = Crypto::ECC::generateKeys();
            _id = Crypto::ECC::NodeIdFactory::fromPublicKey(_keys.publicKey);
        }

        Crypto::ECC::KeyPair getKeys() {
            return _keys;
        }

        NodeId getId() const override {
            return _id;
        }

    private:
        Crypto::ECC::KeyPair _keys;
        NodeId _id;
    };

    typedef std::shared_ptr<HostKeyPair> HostKeyPairPtr;
}

#endif //P2P_MSG_HOSTID_H
