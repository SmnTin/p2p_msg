#include "p2p/P2P.h"

#include "p2p/platform.h"

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <unistd.h>
#elif PLATFORM == PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace p2p {

	P2P::P2P(uint16_t port) : _udpPort(port) {}

	P2P::~P2P() {
	    quit();
	}

	bool P2P::isRunning() {
		return _running;
	}

	uint64_t P2P::getTicks() {
		return _ticks;
	}

	void P2P::update() {
		_sheduler->update();
		++_ticks;
	}

	void P2P::quit() {

		_running = false;
		clean();

		if(_runningThread.joinable())
			_runningThread.join();
	}

	void P2P::handleError(const p2pException & err, const std::string & details) {
		std::cout << "What: " << err.getInfo() << std::endl;
		if(!details.empty()) {
			std::cout << "Additional info: " << details << std::endl;
		}
		std::cout << " Where: " << err.getFile() << "::" << err.getLine() << std::endl;

		_running = false;
	}

	void P2P::sleep(uint32_t sleepTime_ms) {
		std::chrono::duration<long long, std::milli> sleepDur {sleepTime_ms};
	    std::this_thread::sleep_for(sleepDur);
	}

	void P2P::run() {
		if(isRunning())
			return;
		_runningThread = std::thread(_runable(*this));
	}

	void P2P::printStats() {
		std::cout << *_dht->getRoutingTable();
	}

	P2P::_runable::_runable (P2P & instance) : _instance(instance) {}

	void P2P::_runable::operator()() {
		_instance.init();
	}

	void P2P::init() {
		_networkEngine = std::make_shared<Network::NetworkEngine>(_io, _udpPort);
		_sheduler = std::make_shared<Sheduler>();
		_dht = std::make_shared<DHT::DHT>(_networkEngine, _sheduler);
		_rudpEngine = std::make_shared<Network::RUDP::Engine>(_networkEngine, _dht, _sheduler);

		_running = true;
		_sheduler->run();
        _io.run();
	}

	void P2P::clean() {
		_io.stop();
		_sheduler->stop();
	}

	void P2P::connect(boost::asio::ip::udp::endpoint endp) {
		_dht->pingNode(endp);
	}

	void P2P::findNode(const NodeId &nodeId, const DHT::NodeSearch::Callback &callback) {
		if(!callback)
			throw;
		_dht->findNode(nodeId, callback);
	}

	void P2P::storeValue(const NodeId &nodeId, const NodeId &key, const DHT::StorageValueBuffer &val) {
		_dht->storeValue(nodeId, key, val);
	}

	void P2P::getValue(const p2p::NodeId &nodeId, const NodeId &key, const DHT::GetValueRequest::Callback &callback) {
		_dht->getValue(nodeId, key, callback);
	}

}
