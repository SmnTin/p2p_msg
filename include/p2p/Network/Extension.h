#ifndef P2P_MSG_IEXTENSION_H
#define P2P_MSG_IEXTENSION_H

#include "p2p/Network/Stream.h"

namespace p2p::Network {
    class IExtension;

    typedef std::shared_ptr<IExtension> IExtensionPtr;
    typedef std::weak_ptr<IExtension> IExtensionWPtr;

    class IExtension : public std::enable_shared_from_this<IExtension> {
    public:
        virtual void append(IExtensionPtr child) = 0;

        virtual void setParent(IExtensionPtr parent) = 0;

        virtual void extendStream(IStreamPtr stream) = 0;
    };

}

#endif //P2P_MSG_IEXTENSION_H
