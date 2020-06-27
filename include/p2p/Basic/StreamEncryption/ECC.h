#ifndef P2P_MSG_STREAM_ENCRYPTION_ECC_H
#define P2P_MSG_STREAM_ENCRYPTION_ECC_H

#include "p2p/Basic/Network/Extension.h"
#include "p2p/Basic/Network/Stream.h"
#include "p2p/Crypto/ECC.h"
#include "p2p/Crypto/AES.h"
#include "p2p/Basic/Host/HostKeyPair.h"

///TODO: TESTS!!!

namespace p2p::Basic::Network {
    class ECCStream : public Stream<> {
    public:
        explicit ECCStream(Crypto::ECC::PrivateKey hostPrivateKey) {
            Crypto::ECC::PublicKey nodePublicKey = getNodeId().data();
            _sharedKey = Crypto::ECC::generateSharedKey(
                    Crypto::ECC::KeyPair{
                            nodePublicKey,
                            hostPrivateKey
                    });
        }

        void receive(Buffer msg) override {
            auto decrypted = Crypto::AES::decryptECB(msg, _sharedKey);
            if (_child)
                _child->receive(decrypted);
        }

        void send(Buffer msg) override {
            auto encrypted = Crypto::AES::encryptECB(msg, _sharedKey);
            if (auto parent = _parent.lock())
                parent->send(encrypted);
        }

    private:
        Crypto::ECC::SharedKey _sharedKey;
    };

    class ECCExtension : public Extension {
    public:
        explicit ECCExtension(HostKeyPairPtr hostKeyPair) {
            _hostPrivateKey = hostKeyPair->getKeys().privateKey;
        }

        void extendStream(IStreamPtr stream) override {
            IStreamPtr extended =
                    std::make_shared<ECCStream>(_hostPrivateKey);
            extended->append(stream);
            if (_child)
                _child->extendStream(extended);
        }

    private:
        Crypto::ECC::PrivateKey _hostPrivateKey;
    };
}

#endif //P2P_MSG_STREAM_ENCRYPTION_ECC_H
