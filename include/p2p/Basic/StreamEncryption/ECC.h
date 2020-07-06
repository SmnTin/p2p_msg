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
            _hostPrivateKey = hostPrivateKey;
        }

        void receive(Buffer msg) override {
            _generateKey();
            if (_child) {
                auto len = _extractMsgSize(msg);
                auto decrypted = Crypto::AES::decryptECB(_extractMsg(msg),
                                                         _sharedKey.value())
                        .substr(0, len);
                _child->receive(decrypted);
            }
        }

        void send(Buffer msg) override {
            _generateKey();
            if (auto parent = _parent.lock()) {
                auto encrypted = Crypto::AES::encryptECB(msg, _sharedKey.value());
                parent->send(_putMsgSize(encrypted, msg.size()));
            }
        }

    private:
        typedef uint32_t SizeT;

        void _generateKey() {
            if (_sharedKey.has_value())
                return;
            Crypto::ECC::PublicKey nodePublicKey = getNodeId().data();
            _sharedKey = Crypto::ECC::generateSharedKey(
                    Crypto::ECC::KeyPair{
                            nodePublicKey,
                            _hostPrivateKey
                    });
        }

        SizeT _extractMsgSize(Buffer msg) {
            if (msg.size() < sizeof(SizeT))
                throw_p2p_exception("The received message is too short to contain a 4-byte integer.");
            SizeT val = *reinterpret_cast<SizeT *>(msg.data());
            val = networkToHostByteOrder(val);
            return val;
        }

        Buffer _extractMsg(Buffer msg) {
            return msg.substr(sizeof(SizeT));
        }

        Buffer _putMsgSize(Buffer msg, SizeT size) {
            auto net = hostToNetworkByteOrder(size);
            auto raw = reinterpret_cast<uint8_t *>(&net);
            auto enc = Buffer(raw, raw + sizeof(SizeT));
            return enc + msg;
        }

        Crypto::ECC::PrivateKey _hostPrivateKey;
        std::optional<Crypto::ECC::SharedKey> _sharedKey;
    };

    class ECCExtension : public Extension {
    public:
        explicit ECCExtension(HostKeyPairPtr hostKeyPair) {
            _hostPrivateKey = hostKeyPair->getKeys().privateKey;
        }

        void extendStream(IStreamPtr stream) override {
            IStreamPtr extended =
                    std::make_shared<ECCStream>(_hostPrivateKey);
            stream->append(extended);
            if (_child)
                _child->extendStream(extended);
        }

    private:
        Crypto::ECC::PrivateKey _hostPrivateKey;
    };
}

#endif //P2P_MSG_STREAM_ENCRYPTION_ECC_H
