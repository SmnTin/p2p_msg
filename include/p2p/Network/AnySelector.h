#ifndef P2P_MSG_STREAMSTORE_H
#define P2P_MSG_STREAMSTORE_H

#include "p2p/Network/Stream.h"
#include "p2p/Network/Transport.h"

#include <vector>
#include <unordered_map>

namespace p2p::Network {
    template<class T>
    class Filter {
    public:
        typedef std::shared_ptr<T> AnyPtr;

        static bool filter(const AnyPtr &ptr) {};

        template<class A, class B>
        class And : public Filter {
        public:
            static bool filter(const AnyPtr &ptr) {
                if (!A::filter(ptr))
                    return false;
                if (!B::filter(ptr))
                    return false;
                return true;
            }

        private:
            And() = default;
        };

        template<class A, class B>
        class Or : public Filter {
        public:
            static bool filter(const AnyPtr &ptr) {
                if (A::filter(ptr))
                    return true;
                if (B::filter(ptr))
                    return true;
                return false;
            }

        private:
            Or() = default;
        };

        template<class A>
        class Not : public Filter {
        public:
            static bool filter(const AnyPtr &ptr) {
                return !A::filter(ptr);
            }

        private:
            Not() = default;
        };

        class Fast : public Filter {
        public:
            static bool filter(const AnyPtr &ptr) {
                return ptr->getTraits().isFast();
            }

        private:
            Fast() = default;
        };

        typedef Not<Fast> Slow;

        class Reliable : public Filter {
        public:
            static bool filter(const AnyPtr &ptr) {
                return ptr->getTraits().isReliable();
            }

        private:
            Reliable() = default;
        };

        typedef Not<Reliable> Unreliable;

        class EnergyEfficient : public Filter {
        public:
            static bool filter(const AnyPtr &ptr) {
                return ptr->getTraits().isEnergyEfficient();
            }

        private:
            EnergyEfficient() = default;
        };

        typedef Not<EnergyEfficient> EnergyInefficient;

        class Any : public Filter {
        public:
            static bool filter(const AnyPtr &ptr) {
                return true;
            }

        private:
            Any() = default;
        };

        typedef Not<Any> None;
    };

    typedef Filter<ITransport> TransportFilter;
    typedef Filter<IStream> StreamFilter;

    template<class T>
    class AnySelector {
    public:
        typedef std::shared_ptr<T> AnyPtr;
        typedef std::weak_ptr<T> AnyWPtr;
        typedef std::vector<AnyWPtr> PartialContainer;
        typedef std::unordered_map<Buffer, AnyWPtr> AllContainer;

        AnyPtr any() const;
        AnyPtr any();

        AnyPtr anyReliable() const;
        AnyPtr anyReliable();

        AnyPtr anyUnreliable() const;
        AnyPtr anyUnreliable();

        AnyPtr anyEnergyEfficient() const;
        AnyPtr anyEnergyEfficient();

        AnyPtr anyEnergyInefficient() const;
        AnyPtr anyEnergyInefficient();

        AnyPtr anyFast() const;
        AnyPtr anyFast();

        AnyPtr anySlow() const;
        AnyPtr anySlow();

        AnyPtr operator[](const std::string &name) const;
        AnyPtr operator[](const std::string &name);

        typename AllContainer::iterator begin();
        typename AllContainer::iterator end();

        typename AllContainer::const_iterator begin() const;
        typename AllContainer::const_iterator end() const;

        template<class ConcreteFilter>
        AnyPtr query() {
            for (auto it = _all.begin(); it != _all.end();) {
                if (it->second.expired()) {
                    auto ne = std::next(it);
                    _all.erase(it);
                    it = ne;
                } else {
                    auto ptr = it->second.lock();
                    if (ConcreteFilter::filter(ptr))
                        return it->second.lock();
                    else
                        ++it;
                }
            }
            throw std::out_of_range("No such transport.");
        }

        template<class ConcreteFilter>
        AnyPtr query() const {
            for (auto it = _all.begin(); it != _all.end(); ++it)
                if (auto ptr = it->second.lock())
                    if (ConcreteFilter::filter(ptr))
                        return it->second.lock();
            throw std::out_of_range("No such transport.");
        }

        void add(AnyPtr ptr);

    private:
        AllContainer _all;
    };

    typedef AnySelector<ITransport> TransportSelector;
    typedef AnySelector<IStream> StreamSelector;

    class StreamStore : public std::unordered_map<NodeId, StreamSelector> {
    public:
        const StreamSelector &operator[](const NodeId &id) const {
            return at(id);
        }

        StreamSelector &operator[](const NodeId &id) {
            return std::unordered_map<NodeId, StreamSelector>::operator[](id);
        }
    };
}

#endif //P2P_MSG_STREAMSTORE_H
