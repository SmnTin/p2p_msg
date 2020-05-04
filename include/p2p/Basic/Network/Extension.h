#ifndef P2P_MSG_BASIC_EXTENSION_H
#define P2P_MSG_BASIC_EXTENSION_H

namespace p2p::Basic::Network {
    class Extension {
    public:
        virtual void append(IExtensionPtr child) {
            child->setParent(shared_from_this());
            _children.emplace_back(std::move(child));
        }

        virtual void setParent(IExtensionPtr parent) {
            _parent = IExtensionWPtr(std::move(parent));
        }

    private:
        IExtensionWPtr _parent;
        std::vector<IExtensionPtr> _children;
    };
}

#endif //P2P_MSG_BASIC_EXTENSION_H
