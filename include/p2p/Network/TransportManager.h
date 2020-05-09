#ifndef P2P_MSG_NETWORK_TRANSPORTMANAGER_H
#define P2P_MSG_NETWORK_TRANSPORTMANAGER_H

#include "p2p/Network/AnySelector.h"
#include "p2p/Network/Extension.h"

namespace p2p::Network {
    class TransportManager {
    public:
        TransportManager();
        TransportManager(const TransportManager &) = delete;
        TransportManager(TransportManager &&) = delete;

        void add(ITransportPtr transport);

        void extend(IExtensionPtr extension);

        const TransportSelector &select();
    private:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

    typedef std::shared_ptr<TransportManager> TransportManagerPtr;
}

#endif //P2P_MSG_NETWORK_TRANSPORTMANAGER_H
