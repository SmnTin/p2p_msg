#ifndef P2P_MSG_NETWORK_ENDPOINT_H
#define P2P_MSG_NETWORK_ENDPOINT_H

#include "p2p/Buffer.h"

namespace p2p::Network {

    struct Endpoint {
        std::string transportName;
        Buffer data;

        bool operator==(const Endpoint &b) const {
            return transportName == b.transportName &&
                   data == b.data;
        }

        bool operator!=(const Endpoint &b) const {
            return !(*this == b);
        }
    };

    class TransportTraits {
    public:
        TransportTraits(std::string name, bool reliable,
                        bool fast, bool energyEfficient) {
            _name = std::move(name);
            _isReliable = reliable;
            _isFast = fast;
            _isEnergyEfficient = energyEfficient;
        }

        [[nodiscard]] std::string getName() const {
            return _name;
        }

        [[nodiscard]] bool isReliable() const {
            return _isReliable;
        }

        [[nodiscard]] bool isFast() const {
            return _isFast;
        }

        [[nodiscard]] bool isEnergyEfficient() const {
            return _isEnergyEfficient;
        }

        bool operator==(const TransportTraits &b) const {
            return _name == b._name &&
                   _isReliable == b._isReliable &&
                   _isFast == b._isFast &&
                   _isEnergyEfficient == b._isEnergyEfficient;
        }

        bool operator!=(const TransportTraits &b) const {
            return !(*this == b);
        }

    private:
        Buffer _name = "";
        bool _isReliable = false;
        bool _isFast = false;
        bool _isEnergyEfficient = false;
    };

}

#endif //P2P_MSG_NETWORK_ENDPOINT_H
