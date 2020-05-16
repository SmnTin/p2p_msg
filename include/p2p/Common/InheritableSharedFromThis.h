#ifndef P2P_MSG_INHERITABLE_SHARED_FROM_THIS_H
#define P2P_MSG_INHERITABLE_SHARED_FROM_THIS_H

#include <memory>
#include <utility>

namespace std {
    template< class T, class U >
    inline std::shared_ptr<T> reinterpret_pointer_cast(const std::shared_ptr<U>& r) noexcept {
        auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type*>(r.get());
        return std::shared_ptr<T>(r, p);
    }

    template< class T, class U >
    inline std::shared_ptr<T> reinterpret_pointer_cast(std::shared_ptr<U>&& r) noexcept {
        auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type*>(r.get());
        return std::shared_ptr<T>(std::move(r), p);
    }
}

///TODO: get rid of dynamic cast

namespace p2p {
    namespace _ {
        class MultipleInheritableEnableSharedFromThis
                : public std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis> {
        public:
            virtual ~MultipleInheritableEnableSharedFromThis() = default;
        };
    }

    template<class T>
    class inheritable_enable_shared_from_this
            : virtual public _::MultipleInheritableEnableSharedFromThis {
    public:
        std::shared_ptr<T> shared_from_this() {
            return std::dynamic_pointer_cast<T>(
                    MultipleInheritableEnableSharedFromThis::shared_from_this());
        }

        std::weak_ptr<T> weak_from_this() {
            return shared_from_this();
        }
    };
}

#endif //P2P_MSG_INHERITABLE_SHARED_FROM_THIS_H
