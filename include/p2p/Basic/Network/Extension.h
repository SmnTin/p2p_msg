#ifndef P2P_MSG_BASIC_EXTENSION_H
#define P2P_MSG_BASIC_EXTENSION_H

namespace p2p::Basic::Network {
    class Extension {
    public:
        void append(IExtensionPtr child) override {
            child->setParent(shared_from_this());
            _children.emplace_back(std::move(child));
        }

        void setParent(IExtensionPtr parent) override {
            _parent = IExtensionWPtr(parent);
        }

        void extendStream(IStreamPtr stream) override {
            for (auto &child : _children)
                child->extendStream(stream);
        }

    private:
        IExtensionWPtr _parent;
        std::vector<IExtensionPtr> _children;
    };
}

#endif //P2P_MSG_BASIC_EXTENSION_H
