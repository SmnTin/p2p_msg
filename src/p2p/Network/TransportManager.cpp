#include "p2p/Network/TransportManager.h"

#include "p2p/Basic/Network/Extension.h"

namespace p2p::Network {
    class TransportManager::Impl {
    public:
        Impl() {
            _rootExtension = std::make_shared<Basic::Network::Extension>();
        }

        void add(ITransportPtr transport) {
            transport->extend(_rootExtension);
            _selector.add(std::move(transport));
        }

        void extend(IExtensionPtr extension) {
            _rootExtension->setChild(std::move(extension));
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
        _impl->add(transport);
    }

    void TransportManager::extend(IExtensionPtr extension) {
        _impl->extend(extension);
    }

    const TransportSelector& TransportManager::select() {
        return _impl->select();
    }
}