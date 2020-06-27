#include "p2p/Basic/Routers/ProtocolRouter.h"

#include "p2p/Common/Exceptions.h"

namespace p2p::Basic::Network {
    const bool ProtocolParams::Version::operator<(const ProtocolParams::Version &b) const {
        if (major != b.major)
            return major < b.major;
        else if (minor != b.minor)
            return minor < b.minor;
        else
            return revision < b.revision;
    }

    const bool ProtocolParams::Version::operator>(const ProtocolParams::Version &b) const {
        return b < *this;
    }

    const bool ProtocolParams::Version::operator==(const ProtocolParams::Version &b) const {
        return major == b.major &&
               minor == b.minor &&
               revision == b.revision;
    }

    const bool ProtocolParams::Version::operator!=(const ProtocolParams::Version &b) const {
        return !(*this == b);
    }

    const bool ProtocolParams::Version::operator<=(const ProtocolParams::Version &b) const {
        return (*this < b || *this == b);
    }

    const bool ProtocolParams::Version::operator>=(const ProtocolParams::Version &b) const {
        return (*this > b || *this == b);
    }

    ProtocolRouterStream::InternalStream::InternalStream
            (ProtocolRouterStreamWPtr parent, ProtocolParams params) {
        _internalParent = parent;
        _parent = _internalParent;
        _params = std::move(params);
    }

    void ProtocolRouterStream::InternalStream::setParent(IStreamPtr) {
        throw NotImplementedException();
    }

    void ProtocolRouterStream::setChild(IStreamPtr) {
        throw NotImplementedException();
    }

    void ProtocolRouterStream::append(IStreamPtr) {
        throw NotImplementedException();
    }

    void ProtocolRouterStream::send(Buffer) {
        throw NotImplementedException();
    }

    const ProtocolParams &ProtocolRouterStream::InternalStream::getParams() {
        return _params;
    }

    void ProtocolRouterStream::InternalStream::send(Buffer msg) {
        if (auto parent = _internalParent.lock())
            parent->sendFromProtocol(this, msg);
    }

    IStreamPtr ProtocolRouterStream::registerProtocol
            (const ProtocolParams &params) {
        auto _this = inheritable_enable_shared_from_this<ProtocolRouterStream>::weak_from_this();
        auto internal = std::make_shared<InternalStream>(_this, params);
        p2pAssert(!_protocols.count(params.getName()), "Protocol can't be registered twice.");
        _protocols[params.getName()] = internal;
        return internal;
    }

    void ProtocolRouterStream::receive(Buffer msg) {
        std::size_t headerLen;
        auto header = decodeData(msg, headerLen);

        auto it = _protocols.find(header.getName());
        p2pAssert(it != _protocols.end(), "Protocol must exist.");

        auto protocol = it->second;
        p2pAssert(protocol->getParams().getLeastCompatible() <= header.getCurrent(),
                  "Unsupported protocol version.");

        protocol->receive(msg.substr(headerLen));
    }

    void ProtocolRouterStream::sendFromProtocol(InternalStream *stream,
                                                Buffer msg) {
        if (auto parent = _parent.lock())
            parent->send(encodeData(stream->getParams(), msg));
    }

    //message prefix is simple as fuck: /name/1.0.0/
    ProtocolParams ProtocolRouterStream::decodeData(Buffer msg, std::size_t &headerLen) const {
        std::regex re("^\\/([a-zA-Z_\\-.0-9]*)\\/([0-9]{1,8})\\.([0-9]{1,8})\\.([0-9]{1,8})\\/");
        std::smatch match;
        if (std::regex_search(msg, match, re)) {
            std::string name = match[1];
            uint32_t major = std::stoi(match[2]);
            uint32_t minor = std::stoi(match[3]);
            uint32_t rev = std::stoi(match[4]);
            ProtocolParams::Version version{major, minor, rev};
            headerLen = static_cast<std::string>(match[0]).size();
            return ProtocolParams(name, version);
        } else {
            throw std::runtime_error("No correct header.");
        }
    }

    Buffer ProtocolRouterStream::encodeData(const ProtocolParams &params, Buffer msg) const {
        std::string header = "/" + params.getName() + "/";
        header += std::to_string(params.getCurrent().major) + ".";
        header += std::to_string(params.getCurrent().minor) + ".";
        header += std::to_string(params.getCurrent().revision) + "/";
        return header + msg;
    }

    void ProtocolRouterExtension::extendStream(IStreamPtr stream) {
        auto router = std::make_shared<ProtocolRouterStream>();

        for (auto &[name, p] : _protocols) {
            auto &[params, protocol] = p;
            protocol->extendStream(router->registerProtocol(params));
        }
    }

    void ProtocolRouterExtension::registerProtocol(IExtensionPtr extension,
                                                   const ProtocolParams &params) {
        p2pAssert(!_protocols.count(params.getName()), "Protocol can't be registered twice.");
        _protocols[params.getName()] = {params, extension};
    }

    void ProtocolRouterExtension::append(IExtensionPtr) {
        throw NotImplementedException();
    }

    void ProtocolRouterExtension::setChild(IExtensionPtr) {
        throw NotImplementedException();
    }
}