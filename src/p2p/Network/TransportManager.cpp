#include "p2p/Network/TransportManager.h"

#include "p2p/Basic/Network/Extension.h"

namespace p2p::Network {
    class TransportManager::Impl {
    public:
        Impl() {
            _rootExtension = std::make_shared<Basic::Network::RootExtension>();
        }

        void add(ITransportPtr transport) {
            transport->extend(_rootExtension);
            _selector.add(std::move(transport));
        }

        void extend(IExtensionPtr extension) {
            _rootExtension->append(std::move(extension));
        }

        const TransportSelector &select() {
            return _selector;
        }
    private:
        TransportSelector _selector;
        IExtensionPtr _rootExtension;
    };

    TransportManager::TransportManager() {
        _impl = std::make_unique<Impl>();
    }

    void TransportManager::add(ITransportPtr transport) {
        _impl->add(std::move(transport));
    }

    void TransportManager::extend(IExtensionPtr extension) {
        _impl->extend(std::move(extension));
    }

    const TransportSelector& TransportManager::select() {
        return _impl->select();
    }
}