#ifndef P2P_MSG_BASIC_EXTENSION_H
#define P2P_MSG_BASIC_EXTENSION_H

#include "p2p/Network/Extension.h"
#include "p2p/Common/Exceptions.h"

namespace p2p::Basic::Network {
    using namespace p2p::Network;

    class Extension : public IExtension {
    public:
        void append(IExtensionPtr child) override {
            if (_child)
                _child->setParent(nullptr);
            if (child)
                child->setParent(shared_from_this());
            setChild(child);
        }

        void setChild(IExtensionPtr child) override {
            _child = std::move(child);
        }

        void setParent(IExtensionPtr parent) override {
            if (parent)
                _parent = IExtensionWPtr(parent);
            else
                _parent = IExtensionWPtr();
        }

        void extendStream(IStreamPtr stream) override {
            if (_child)
                _child->extendStream(stream);
        }

    protected:
        IExtensionWPtr _parent;
        IExtensionPtr _child;
    };

    class RootExtension : public Extension {
    public:
        void setParent(IExtensionPtr parent) override {
            throw NotImplementedException();
        }
    };
}

#endif //P2P_MSG_BASIC_EXTENSION_H
