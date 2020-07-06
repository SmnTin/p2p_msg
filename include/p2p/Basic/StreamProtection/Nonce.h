#ifndef P2P_MSG_NONCE_H
#define P2P_MSG_NONCE_H

#include "p2p/Basic/Network/NonceChecker.h"
#include "p2p/Basic/Network/Stream.h"
#include "p2p/Basic/Network/Extension.h"
#include "p2p/Common/Endianness.h"

#include <cstring>

namespace p2p::Basic::Network {
    template<typename Nonce, size_t queueSize>
    class StreamWithNonce : public Stream<> {
    public:
        StreamWithNonce() = default;

        void receive(Buffer msg) override {
            checkMsgForCorrectness(msg);
            Nonce nonce = extractNonce(msg);
            if (_checker.checkAndUpdate(nonce) && _child)
                _child->receive(extractMessage(msg));
        }

        void send(Buffer msg) override {
            if (auto parent = _parent.lock())
                parent->send(putNonceInMsg(msg, _thisNonce++));
        }

    private:
        void checkMsgForCorrectness(Buffer msg) {
            if (msg.size() < sizeof(Nonce))
                throw_p2p_exception("Message is too little to contain nonce.");
        }

        Nonce extractNonce(Buffer msg) {
            Nonce nonceInNetworkByteOrder = *reinterpret_cast<Nonce *>(msg.data());
            return networkToHostByteOrder(nonceInNetworkByteOrder);
        }

        Buffer extractMessage(Buffer msg) {
            return msg.substr(sizeof(Nonce));
        }

        Buffer putNonceInMsg(Buffer msg, Nonce nonce) {
            Nonce nonceInNetworkByteOrder = hostToNetworkByteOrder(nonce);
            Buffer nonceStr(4, '\0');
            memcpy(nonceStr.data(), &nonceInNetworkByteOrder, sizeof(Nonce));
            return nonceStr + msg;
        }

        Nonce _thisNonce = 0u;
        NonceChecker<Nonce, queueSize> _checker;
    };

    template<typename Nonce, size_t queueSize>
    class ExtensionWithNonce : public Extension {
    public:
        void extendStream(IStreamPtr stream) override {
            auto extended = std::make_shared<StreamWithNonce<Nonce, queueSize>>();
            stream->append(extended);
            if (_child)
                _child->extendStream(extended);
        }
    };
}

#endif //P2P_MSG_NONCE_H
