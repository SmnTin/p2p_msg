#ifndef P2P_MSG_ACCOUNT_H
#define P2P_MSG_ACCOUNT_H

#include "p2p/Node.h"
#include "p2p/Network/RUDP/Engine.h"
#include "p2p/DHT/DHT.h"
#include "p2p/Messenger/Friend.h"

#include <cryptopp/eccrypto.h>

#include <vector>

namespace p2p {
    namespace Messenger {

        class Account {
        public:
            explicit Account(
                    std::shared_ptr<Network::RUDP::Engine> engine,
                    std::shared_ptr<DHT::DHT> dht,
                    std::shared_ptr<Sheduler> sheduler);
            ~Account();

            void setPrivateKey(const PrivateKey & privateKey);
            void setPrivateKey(const std::string & str);

            std::string getPrivateKeyAsString();
            std::string getPublicKeyAsString();

            std::shared_ptr<Friend> findFriend(const std::string &key);

            void addFriend(const std::string &key);
            void addFriend(const std::string &key, const std::shared_ptr<Network::RUDP::Connection> &conn);
            void removeFriend(const std::string &key);

            void generateKeys();

        private:
            PrivateKey _privateKey;
            PublicKey _publicKey;
            std::shared_ptr<Network::RUDP::Engine> _rudpEngine;
            std::shared_ptr<DHT::DHT> _dht;
            std::shared_ptr<Sheduler> _sheduler;

            std::vector<std::shared_ptr<Friend>> _friends;
            std::recursive_mutex _friendsMutex;

            std::shared_ptr<Task> _friendsJob;

            void friendsTask();
            void friendTaskP1(std::shared_ptr<Friend> fr, NodeId frNodeId, NodeId myNodeId, std::shared_ptr<Node> node);
            void friendTaskP2(std::shared_ptr<Friend> fr, NodeId frNodeId, NodeId myNodeId, bool success, DHT::StorageValueBuffer val);
            void friendTaskP3(std::shared_ptr<Friend> fr, NodeId frNodeId, NodeId myNodeId, std::shared_ptr<Node> node);
            void friendTaskP4(std::shared_ptr<Friend> fr, NodeId frNodeId, NodeId myNodeId, std::shared_ptr<Node> node);
        };
    } //Messenger
} //p2p

#endif //P2P_MSG_ACCOUNT_H
