#ifndef P2P_MSG_FRIEND_H
#define P2P_MSG_FRIEND_H

#include <cryptopp/eccrypto.h>
#include <cryptopp/hex.h>
#include <cryptopp/oids.h>
#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <cryptopp/files.h>

#include "p2p/Node.h"

namespace p2p {
    namespace Messenger {

        typedef CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey PublicKey;
        typedef CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey PrivateKey;

        class Friend {
        public:
            Friend() {

            }

            void setPublicKey(const PublicKey & publicKey) {
                _publicKey = publicKey;
            }

            void setPublicKey(const std::string & publicKeyStr) {
                CryptoPP::HexDecoder decoder;
                decoder.Put((CryptoPP::byte*)&publicKeyStr[0], publicKeyStr.size());
                decoder.MessageEnd();

                CryptoPP::ECP::Point q;
                size_t len = decoder.MaxRetrievable();
                q.identity = false;
                q.x.Decode(decoder, len/2);
                q.y.Decode(decoder, len/2);

                _publicKey.Initialize(CryptoPP::ASN1::secp256r1(), q);
            }

            std::string getPublicKeyAsString() {
                std::string res;
                std::stringstream ss;
                ss << std::hex << _publicKey.GetPublicElement().x << _publicKey.GetPublicElement().y;
                ss >> res;
                return res;
            }

            void setConnection(const std::shared_ptr<Network::RUDP::Connection> &conn) {
                _conn = conn;
            }

            std::shared_ptr<Network::RUDP::Connection> getConnection() {
                return _conn;
            }

        private:
            PublicKey _publicKey;

            std::shared_ptr<Network::RUDP::Connection> _conn = nullptr;
        };
    } //Messenger
} //p2p

#endif //P2P_MSG_FRIEND_H
