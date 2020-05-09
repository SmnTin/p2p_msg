#include "p2p/Network/AnySelector.h"

namespace p2p::Network {
    template<class T>
    typename AnySelector<T>::AllContainer::iterator
    AnySelector<T>::begin() {
        return _all.begin();
    }

    template<class T>
    typename AnySelector<T>::AllContainer::const_iterator
    AnySelector<T>::begin() const {
        return _all.begin();
    }

    template<class T>
    typename AnySelector<T>::AllContainer::iterator
    AnySelector<T>::end() {
        return _all.end();
    }

    template<class T>
    typename AnySelector<T>::AllContainer::const_iterator
    AnySelector<T>::end() const {
        return _all.end();
    }

    template<class T>
    void AnySelector<T>::add(AnyPtr ptr) {
        auto it = _all.find(ptr->getTraits().getName());
        if (it == _all.end() || it->second.expired())
            _all[ptr->getTraits().getName()] = ptr;
        else
            throw std::runtime_error
                    ("A transport with this name is "
                     "already presented in the table.");
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::operator[](const Buffer &name) {
        auto it = _all.find(name);
        if (it == _all.end() || it->second.expired()) {
            if (it != _all.end())
                _all.erase(it);
            throw std::out_of_range("No such transport.");
        }

        return it->second.lock();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::operator[](const Buffer &name) const {
        auto it = _all.find(name);
        if (it == _all.end() || it->second.expired())
            throw std::out_of_range("No such transport.");

        return it->second.lock();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::any() {
        return query<typename Filter<T>::Any>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::any() const {
        return query<typename Filter<T>::Any>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyReliable() {
        return query<typename Filter<T>::Reliable>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyReliable() const {
        return query<typename Filter<T>::Reliable>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyUnreliable() {
        return query<typename Filter<T>::Unreliable>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyUnreliable() const {
        return query<typename Filter<T>::Unreliable>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyFast() {
        return query<typename Filter<T>::Fast>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyFast() const {
        return query<typename Filter<T>::Fast>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anySlow() {
        return query<typename Filter<T>::Slow>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anySlow() const {
        return query<typename Filter<T>::Slow>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyEnergyEfficient() {
        return query<typename Filter<T>::EnergyEfficient>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyEnergyEfficient() const {
        return query<typename Filter<T>::EnergyEfficient>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyEnergyInefficient() {
        return query<typename Filter<T>::EnergyInefficient>();
    }

    template<class T>
    typename AnySelector<T>::AnyPtr
    AnySelector<T>::anyEnergyInefficient() const {
        return query<typename Filter<T>::EnergyInefficient>();
    }

    template
    class AnySelector<IStream>;

    template
    class AnySelector<ITransport>;
}