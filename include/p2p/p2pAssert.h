#ifndef P2P_P2PASSERT_H
#define P2P_P2PASSERT_H

#include <string>

//#include "errors.h"
#include "p2p/p2pException.h"

namespace p2p {
	void p2pAssert(const bool & condition, const std::string & details);
}

#endif
