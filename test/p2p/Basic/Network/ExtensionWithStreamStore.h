#ifndef P2P_MSG_EXTENSIONWITHSTREAMSTORE_H
#define P2P_MSG_EXTENSIONWITHSTREAMSTORE_H

#include "p2p/Basic/Network/Extension.h"
#include "p2p/Network/AnySelector.h"

namespace p2p::Basic::Network {
    class ExtensionWithStreamStore : public Extension {
    public:
        StreamStore &getStore() {
            return _store;
        }

        void extendStream(IStreamPtr stream) override {
            _store[stream->getNodeId()].add(stream);
            if (_child)
                _child->extendStream(stream);
        }

    private:
        StreamStore _store;
    };
}

#endif //P2P_MSG_EXTENSIONWITHSTREAMSTORE_H