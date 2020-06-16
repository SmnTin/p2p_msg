#ifndef P2P_MSG_COMMON_HOLDER_H
#define P2P_MSG_COMMON_HOLDER_H

#include <memory>

namespace p2p {
    namespace details {
        class BaseHolder {
        public:
            BaseHolder() {}

            virtual ~BaseHolder() = default;
        };

        template<typename T>
        class Holder : public BaseHolder {
        public:
            template<typename Q>
            Holder(Q &&obj) : _object(std::forward<Q>(obj)) {}

            ~Holder() override = default;

        private:
            T _object;
        };
    }

    //type-erased unique_ptr
    class Holder {
    public:
        template<typename Q>
        Holder(Q &&obj) : _holder(std::make_unique<
                details::Holder<std::remove_reference_t<Q>>>
                                          (std::forward<Q>(obj))) {}

    private:
        std::unique_ptr<details::BaseHolder> _holder;
    };
}

#endif //P2P_MSG_COMMON_HOLDER_H
