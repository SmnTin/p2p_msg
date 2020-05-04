#ifndef P2P_MSG_COMMON_SUBSCRIPTION_H
#define P2P_MSG_COMMON_SUBSCRIPTION_H

#include <memory>
#include <functional>
#include <map>
#include <iostream>

namespace p2p {

    struct IPublisher {
        virtual void onUnsubscribe(std::size_t subId) = 0;

        virtual ~IPublisher() {}
    };

    class Subscription {
    public:
        explicit Subscription(std::size_t id,
                              IPublisher *pub,
                              std::shared_ptr<bool> existionMarker)
                : _pubPtr(pub), _active(true), _id(id),
                  _existenceMarker(existionMarker) {}

        void unsubscribe() {
            if (active()) {
                _pubPtr->onUnsubscribe(_id);
                _active = false;
            }
        }

        bool active() const {
            return _active && _pubPtr && !_existenceMarker.expired();
        }

        ~Subscription() {
            unsubscribe();
        }

    private:
        // raw pointer because otherwise it would require
        // std::enable_shared_from_this from Publisher and
        // that results in obligatory storing Publisher in std::shared_ptr
        IPublisher *_pubPtr;
        std::weak_ptr<bool> _existenceMarker;
        bool _active;
        std::size_t _id;
    };

    namespace Basic {
        template<class Data>
        class Publisher : public IPublisher {
        public:
            Publisher() = default;
            Publisher(const Publisher &) {}
            Publisher(Publisher &&) noexcept {}

            typedef std::function<void(Data)> Callback;

            Subscription subscribe(Callback cb) {
                auto id = _subIdCnt++;
                _subs[id] = std::move(cb);

                return Subscription(id, this, _existionMarker);
            }

            void publish(Data data) {
                for (auto &cb : _subs)
                    cb.second(data);
            }

            ~Publisher() override {
            }

        protected:
            void onUnsubscribe(std::size_t subId) override {
                _subs.erase(subId);
            }

            std::shared_ptr<bool> _existionMarker
                    = std::make_shared<bool>(true);

            std::size_t _subIdCnt = 0;
            std::map<std::size_t, Callback> _subs;
        };
    }
}

#endif //P2P_MSG_COMMON_SUBSCRIPTION_H
