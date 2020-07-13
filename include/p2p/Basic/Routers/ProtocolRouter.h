#ifndef P2P_MSG_PROTOCOLROUTER_H
#define P2P_MSG_PROTOCOLROUTER_H

#include "p2p/Basic/Network/Stream.h"
#include "p2p/Basic/Network/Extension.h"

#include <memory>
#include <map>
#include <regex>
#include <utility>

///TODO: performHandshake(), performClosure(), reportThatOpened()

namespace p2p::Basic::Network {
    class ProtocolParams {
    public:
        struct Version {
            uint32_t major = 0;
            uint32_t minor = 0;
            uint32_t revision = 0;

            const bool operator<(const Version &) const;
            const bool operator>(const Version &) const;
            const bool operator<=(const Version &) const;
            const bool operator>=(const Version &) const;
            const bool operator==(const Version &) const;
            const bool operator!=(const Version &) const;
        };

        ProtocolParams() = default;

        ProtocolParams(std::string name, Version current, Version leastCompatible = {0, 0, 0})
                : _name(name), _leastCompatible(leastCompatible), _current(current) {
            std::regex re("^[a-zA-Z_\\-.0-9]*$");
            std::smatch match;
            p2pAssert(std::regex_match(name, match, re), "Protocol must match regex.");
            p2pAssert(leastCompatible <= current, "Least compatible version must be <= than current.");
        }

        std::string getName() const {
            return _name;
        }

        Version getLeastCompatible() const {
            return _leastCompatible;
        }

        Version getCurrent() const {
            return _current;
        }

    private:
        std::string _name;
        //the last version backward compatibility supports
        Version _leastCompatible;
        Version _current;
    };

    class ProtocolRouterStream;

    typedef std::weak_ptr<ProtocolRouterStream> ProtocolRouterStreamWPtr;
    typedef std::shared_ptr<ProtocolRouterStream> ProtocolRouterStreamPtr;

    class ProtocolRouterStream : public Stream<>,
                                 public inheritable_enable_shared_from_this<ProtocolRouterStream> {
    public:
        void setChild(IStreamPtr) override; //prohibited
        void append(IStreamPtr) override; //prohibited

        void receive(Buffer msg) override;
        void send(Buffer) override; //prohibited

        IStreamPtr registerProtocol(const ProtocolParams &params);

    private:
        class InternalStream : public Stream<> {
        public:
            InternalStream(ProtocolRouterStreamWPtr parent, ProtocolParams params);

            void setParent(IStreamPtr) override; //prohibited

            void send(Buffer msg) override;

            const ProtocolParams &getParams();

        private:
            ProtocolRouterStreamWPtr _internalParent;
            ProtocolParams _params;
        };

        typedef std::shared_ptr<InternalStream> InternalStreamPtr;

        void sendFromProtocol(InternalStream *stream, Buffer msg);

        ProtocolParams decodeData(Buffer msg, std::size_t &headerLen) const;
        Buffer encodeData(const ProtocolParams &params, Buffer msg) const;

        std::map<std::string, InternalStreamPtr> _protocols;
    };

    class ProtocolRouterExtension : public Extension {
    public:
        virtual void registerProtocol(IExtensionPtr extension, const ProtocolParams &params);
        void extendStream(IStreamPtr stream) override;

        void append(IExtensionPtr) override; //prohibited
        void setChild(IExtensionPtr) override; //prohibited

    private:
        std::map<std::string, std::pair<ProtocolParams, IExtensionPtr>> _protocols;
    };

    typedef std::weak_ptr<ProtocolRouterExtension> ProtocolRouterExtensionWPtr;
    typedef std::shared_ptr<ProtocolRouterExtension> ProtocolRouterExtensionPtr;

    namespace Builder {
        template<uint32_t major,
                uint32_t minor,
                uint32_t revision>
        class Version {
        public:
            static ProtocolParams::Version get() {
                return {major, minor, revision};
            }
        };

        template<const char *name>
        class Name {
        public:
            static std::string get() {
                return std::string(name);
            }
        };

        template<typename name,
                typename currentVersion,
                typename leastCompatible,
                typename Proto>
        class Protocol {
        public:
            Protocol(std::shared_ptr<Proto> _proto)
                    : proto(_proto) {}

            static ProtocolParams getParams() {
                return ProtocolParams(name::get(), currentVersion::get(), leastCompatible::get());
            }

            IExtensionPtr getProto() {
                return proto;
            }

        private:
            IExtensionPtr proto;
        };

        template<typename ...Args>
        class ProtocolRouter {
        public:
            ProtocolRouter(Args ...extensions) {
                obj = std::make_shared<ProtocolRouterExtension>();
                int dummy[] = {0, (obj->registerProtocol(extensions.getProto(), extensions.getParams()), 0)...};
            }

            operator IExtensionPtr() {
                return obj;
            }

        private:
            ProtocolRouterExtensionPtr obj;
        };
    }
}

#endif //P2P_MSG_PROTOCOLROUTER_H
