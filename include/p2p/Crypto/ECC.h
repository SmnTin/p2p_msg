#ifndef P2P_MSG_CRYPTO_ECC_H
#define P2P_MSG_CRYPTO_ECC_H

#include "p2p/Crypto/Key.h"

namespace p2p::Crypto::ECC {
    typedef Key<32> PublicKey;
    typedef Key<32> PrivateKey;
    typedef Key<32> SharedKey;

    void generateKeys(PublicKey &pubKey, PrivateKey &priKey);
    void generateSharedKey(const PublicKey &pubKey, const PrivateKey &priKey, SharedKey &sharedKey);
}

#endif //P2P_MSG_CRYPTO_ECC_H
