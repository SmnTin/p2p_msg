#include "p2p/Messenger/Account.h"

namespace p2p {
    namespace Messenger {

        void friendTaskP1(Account * acc, std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                          p2p::NodeId myNodeId, std::shared_ptr<p2p::Node> node);
        void friendTaskP2(Account * acc, std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                          p2p::NodeId myNodeId, bool success, p2p::DHT::StorageValueBuffer val);
        void friendTaskP3(Account * acc, std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                          p2p::NodeId myNodeId, std::shared_ptr<p2p::Node> node);
        void friendTaskP4(Account * acc, std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                          p2p::NodeId myNodeId, std::shared_ptr<p2p::Node> node);

        Account::Account(std::shared_ptr<Network::RUDP::Engine> engine,
                         std::shared_ptr<DHT::DHT> dht,
                         std::shared_ptr<Sheduler> sheduler) :
                         _rudpEngine(engine), _dht(dht), _sheduler(sheduler) {

            generateKeys();

//            friendsTask();
        }

        Account::~Account() {
//            std::cout << "WHAT THE FUCK!?\n";
            if(_friendsJob)
                _friendsJob->cancel();
        }

        void Account::setPrivateKey(const p2p::Messenger::PrivateKey &privateKey) {
            _privateKey = privateKey;
            _privateKey.MakePublicKey(_publicKey);
        }

        void Account::setPrivateKey(const std::string &str) {
            CryptoPP::HexDecoder decoder;
            decoder.Put((CryptoPP::byte*)&str[0], str.size());
            decoder.MessageEnd();
            CryptoPP::Integer x;
            x.Decode(decoder, decoder.MaxRetrievable());
            _privateKey.Initialize(CryptoPP::ASN1::secp256r1(), x);
            _privateKey.MakePublicKey(_publicKey);
        }

        std::string Account::getPrivateKeyAsString() {
            std::string res;
            CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(res));

            _privateKey.Save(encoder);
            return res;
        }

        std::string Account::getPublicKeyAsString() {
            std::string res;
            std::stringstream ss;
            ss << std::hex << _publicKey.GetPublicElement().x << _publicKey.GetPublicElement().y;
            ss >> res;
            return res;
        }

        void Account::generateKeys() {
            CryptoPP::AutoSeededRandomPool prng;
            _privateKey.Initialize(prng, CryptoPP::ASN1::secp256r1());
            _privateKey.MakePublicKey(_publicKey);
        }

        std::shared_ptr<Friend> Account::findFriend(const std::string &key) {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            auto it = std::find_if(_friends.begin(), _friends.end(), [key](auto fr) -> bool {
                return fr->getPublicKeyAsString() == key;
            });

            if(it != _friends.end())
                return *it;
            else
                return nullptr;
        }

        void Account::addFriend(const std::string &key) {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            if(findFriend(key) != nullptr)
                return;

            std::shared_ptr<Friend> fr(new Friend());
            fr->setPublicKey(key);
            _friends.push_back(fr);
        }

        void Account::addFriend(const std::string &key, const std::shared_ptr<p2p::Network::RUDP::Connection> &conn) {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            auto exFr = findFriend(key);
            if(exFr != nullptr) {
                exFr->setConnection(conn);
                return;
            }

            std::shared_ptr<Friend> fr(new Friend());
            fr->setPublicKey(key);
            fr->setConnection(conn);
            _friends.push_back(fr);
        }

        void Account::removeFriend(const std::string &key) {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            auto it = std::find_if(_friends.begin(), _friends.end(), [key](auto fr) -> bool {
                return fr->getPublicKeyAsString() == key;
            });
            if(it != _friends.end())
                _friends.erase(it);
        }

        void Account::friendsTask() {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            for(auto & fr : _friends) {
                NodeId frNodeId;
                NodeId myNodeId;

                std::string frNodeIdStr = fr->getPublicKeyAsString();
                frNodeIdStr = frNodeIdStr.substr(frNodeIdStr.size() - NodeId::sizeInBytes);
                std::string myNodeIdStr = getPublicKeyAsString();
                myNodeIdStr = myNodeIdStr.substr(myNodeIdStr.size() - NodeId::sizeInBytes);

                for(int i = 0; i < NodeId::sizeInBytes; ++i)
                    frNodeId[i] = (uint8_t)frNodeIdStr[i];
                for(int i = 0; i < NodeId::sizeInBytes; ++i)
                    myNodeId[i] = (uint8_t)myNodeIdStr[i];


                _dht->findNode(frNodeId, std::bind(&Account::friendTaskP1, this,
                                                           fr, frNodeId, myNodeId, std::placeholders::_1));

                _dht->findNode(myNodeId, std::bind(&Account::friendTaskP4, this,
                                                           fr, frNodeId, myNodeId, std::placeholders::_1));

            }

            _friendsJob = _sheduler->executeTaskIn(std::chrono::milliseconds{PING_DELAY * 1000},
                    std::bind(&Account::friendsTask, this));


        }

        void Account::friendTaskP1(std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                                   p2p::NodeId myNodeId, std::shared_ptr<p2p::Node> node) {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            if(node == nullptr)
                return;


            DHT::GetValueRequest::Callback callback = std::bind(&Account::friendTaskP2, this,
                                                           fr, frNodeId, myNodeId,
                                                           std::placeholders::_1,
                                                           std::placeholders::_2);

            _dht->getValue(node->id(), myNodeId, callback);
        }

        void Account::friendTaskP2(std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                                   p2p::NodeId myNodeId, bool success, p2p::DHT::StorageValueBuffer val) {

            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);
            if(!success)
                return;

            NodeId nodeId;
            for(int i = 0; i < NodeId::sizeInBytes; ++i)
                nodeId[i] = val[i];

            DHT::NodeSearch::Callback callback = std::bind(&Account::friendTaskP3, this,
                                                           fr, frNodeId, myNodeId, std::placeholders::_1);
            _dht->findNode(nodeId, callback);
        }

        void Account::friendTaskP3(std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                                   p2p::NodeId myNodeId, std::shared_ptr<p2p::Node> node) {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            fr->setConnection(_rudpEngine->createConnection(node));
            fr->getConnection()->connect();
        }

        void Account::friendTaskP4(std::shared_ptr<p2p::Messenger::Friend> fr, p2p::NodeId frNodeId,
                                   p2p::NodeId myNodeId, std::shared_ptr<p2p::Node> node) {
            std::lock_guard<std::recursive_mutex> guard(_friendsMutex);

            if(node == nullptr)
                return;

            DHT::StorageValueBuffer val;
            for(int i = 0; i < NodeId::sizeInBytes; ++i)
                val[i] = _dht->getRoutingTable()->getId()[i];

            _dht->storeValue(node->id(), frNodeId, val);
        }
    } //Messenger
} //p2p