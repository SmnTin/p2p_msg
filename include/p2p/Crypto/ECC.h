#ifndef P2P_MSG_CRYPTO_ECC_H
#define P2P_MSG_CRYPTO_ECC_H

#include "p2p/Crypto/Key.h"
#include "p2p/NodeId.h"

#include <future>

namespace p2p::Crypto::ECC {
    typedef Key<32> PublicKey;
    typedef Key<32> PrivateKey;
    typedef Key<32> SharedKey;
    struct KeyPair {
        PublicKey publicKey{0};
        PrivateKey privateKey{0};
    };

    class NodeIdFactory {
    public:
        static NodeId fromPublicKey(const PublicKey &publicKey);

    private:
        NodeIdFactory() = default;
    };

    void generateKeys(PublicKey &pubKey, PrivateKey &priKey);
    KeyPair generateKeys();
    void generateSharedKey(const PublicKey &pubKey, const PrivateKey &priKey, SharedKey &sharedKey);
    SharedKey generateSharedKey(const KeyPair &keyPair);
}

#endif //P2P_MSG_CRYPTO_ECC_H
