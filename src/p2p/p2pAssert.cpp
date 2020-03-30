#include "../../include/p2p/p2pAssert.h"

#include "p2p/P2P.h"

namespace p2p {
	void p2pAssert(const bool & condition, const std::string & details) {
		if(!condition) {
			throw_p2p_exception((((std::string)"Assertion failed! ") + details).c_str());
//			exit(0);
		}
	}
}
