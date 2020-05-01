#ifndef P2P_MSG_NODEREPOSITORY_H
#define P2P_MSG_NODEREPOSITORY_H

#include "p2p/Node.h"
#include <memory>
#include <functional>

//just a usual collection
class NodeRepository {
public:
    struct Iterator
            : public std::iterator<std::forward_iterator_tag,
                    NodePtr> {
        virtual void operator++() = 0;
        virtual void operator++(int) = 0;
        virtual NodePtr operator*() = 0;
        virtual NodePtr operator->() = 0;
    };

    virtual Iterator begin() = 0;
    virtual Iterator end() = 0;

    virtual Iterator find(const NodeId &id) = 0;
    virtual NodePtr findPtr(const NodeId &id) = 0;
    virtual NodePtr operator[](const NodeId &id) = 0;

    virtual bool contains(const NodeId &id) = 0;
    virtual size_t count(const NodeId &id) = 0;

    virtual bool empty() = 0;
    virtual std::size_t size() = 0;

    virtual ~NodeRepository() = default;
};

#endif //P2P_MSG_NODEREPOSITORY_H
