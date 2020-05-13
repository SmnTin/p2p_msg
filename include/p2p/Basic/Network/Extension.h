#ifndef P2P_MSG_BASIC_EXTENSION_H
#define P2P_MSG_BASIC_EXTENSION_H

#include "p2p/Network/Extension.h"

namespace p2p::Basic::Network {
    using namespace p2p::Network;

    class Extension : public IExtension {
    public:
        void setChild(IExtensionPtr child) override {
            if (_child)
                _child->setParent(nullptr);
            if (child)
                child->setParent(shared_from_this());
            _child = std::move(child);
        }

        void setChild(std::nullptr_t child) override {
            setChild(IExtensionPtr(child));
        }

        void setParent(IExtensionPtr parent) override {
            _parent = IExtensionWPtr(parent);
        }

        void setParent(std::nullptr_t parent) override {
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
}

#endif //P2P_MSG_BASIC_EXTENSION_H
