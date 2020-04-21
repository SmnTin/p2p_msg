#include "p2p/Crypto/ECC.h"
#define ECC_CURVE secp256r1
#include "easy-ecc/ecc.h"
#include "p2p/Crypto/Hash.h"

#include <cstring>
#include <thread>

namespace p2p::Crypto::ECC {
    void generateKeys(PublicKey &pubKey, PrivateKey &priKey) {
        uint8_t o_pub[ECC_BYTES + 1];
        uint8_t o_pri[ECC_BYTES];

        do {
            ecc_make_key(o_pub, o_pri);
        } while (o_pub[0] != 0x02);

        memcpy(pubKey.data(), o_pub + 1, ECC_BYTES);
        memcpy(priKey.data(), o_pri, ECC_BYTES);
    }

    KeyPair generateKeys() {
        KeyPair keyPair;
        generateKeys(keyPair.publicKey, keyPair.privateKey);
        return keyPair;
    }

    std::future<KeyPair> generateKeysAsync() {
        return std::async([]() {
            return generateKeys();
        });
    }

    void generateSharedKey(const PublicKey &pubKey, const PrivateKey &priKey, SharedKey &sharedKey) {
        uint8_t o_pub[ECC_BYTES + 1];
        uint8_t o_pri[ECC_BYTES];
        uint8_t o_sha[ECC_BYTES];

        memcpy(o_pub + 1, pubKey.data(), ECC_BYTES);
        memcpy(o_pri, priKey.data(), ECC_BYTES);
        o_pub[0] = 0x02;
        ecdh_shared_secret(o_pub, o_pri, o_sha);
        auto hash = Hash::SHA256(o_sha, ECC_BYTES);
        memcpy(sharedKey.data(), hash.data(), ECC_BYTES);
    }

    SharedKey generateSharedKey(const KeyPair &keyPair) {
        SharedKey key;
        generateSharedKey(keyPair.publicKey, keyPair.privateKey, key);
        return key;
    }

    std::future<SharedKey> generateSharedKeyAsync(const KeyPair &keyPair) {
        return std::async([&keyPair]() {
            return generateSharedKey(keyPair);
        });
    }

    NodeId NodeIdFactory::fromPublicKey(const PublicKey &publicKey) {
        NodeId res;
        for (std::size_t i = 0; i < NodeId::sizeInBytes; ++i)
            res[i] = publicKey[i];
        return res;
    }
}