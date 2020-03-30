#ifndef P2P_MSG_ENGINE_H
#define P2P_MSG_ENGINE_H

#include "p2p/Node.h"
#include "p2p/Network/NetworkEngine.h"
#include "p2p/Network/RUDP/Packet.h"
#include "p2p/Network/RUDP/Connection.h"
#include "p2p/Network/RUDP/RUDPMessageHandlers.h"
#include "p2p/Sheduler.h"
#include "p2p/DHT/DHT.h"

#include <bitset>
#include <unordered_map>

namespace p2p {
    namespace Network {
        namespace RUDP {

            class Engine {
            public:
                explicit Engine(const std::shared_ptr<NetworkEngine> & networkEngine,
                        const std::shared_ptr<DHT::DHT> & dht,
                        const std::shared_ptr<Sheduler> & sheduler) {
                    _networkEngine = networkEngine;
                    _dht = dht;
                    _sheduler = sheduler;

                    _msgHandler = std::make_shared<RUDPMessageHandlers> (this);
                    _networkEngine->registerMessageHandler(_msgHandler);

                    _unusedConnIds.set();
                }

                std::shared_ptr<Connection> createConnection(std::shared_ptr<Node> node) {
                    ConnectionId connId = _unusedConnIds._Find_first();
                    _unusedConnIds.reset(connId);
                    std::shared_ptr<Connection> conn(new Connection(this, connId, node));
                    _conns[connId] = conn;
                    return conn;
                }

                void onConnection() {}
                void onDisconnection(std::shared_ptr<Connection> conn) {
                    _unusedConnIds.set(conn->getConnectionId());
                    _conns.erase(_conns.find(conn->getConnectionId()));
                }

                std::shared_ptr<Connection> operator[] (ConnectionId connId) {
                    auto it = _conns.find(connId);
                    if(it != _conns.end())
                        return it->second;
                    else
                        return nullptr;
                }

                std::shared_ptr<Connection> findConnectionByNodeId(const NodeId & nodeId) {
                    auto it = std::find_if(_conns.begin(), _conns.end(), [nodeId](auto conn) -> bool {
                        return conn.second->getNode()->id() == nodeId;
                    });
                    if(it != _conns.end())
                        return it->second;
                    else
                        return nullptr;
                }

                std::shared_ptr<NetworkEngine> getNetworkEngine() {
                    return _networkEngine;
                }

                std::shared_ptr<Sheduler> getSheduler() {
                    return _sheduler;
                }

                std::shared_ptr<DHT::DHT> getDHT() {
                    return _dht;
                }

            private:
                std::shared_ptr<NetworkEngine> _networkEngine;
                std::shared_ptr<DHT::DHT> _dht;
                std::shared_ptr<RUDPMessageHandlers> _msgHandler;
                std::bitset<MAX_CONNECTIONS> _unusedConnIds;
                std::unordered_map<ConnectionId, std::shared_ptr<Connection>> _conns;
                std::shared_ptr<Sheduler> _sheduler;
            };
        } //RUDP
    } //Network
} //p2p

#endif //P2P_MSG_ENGINE_H
