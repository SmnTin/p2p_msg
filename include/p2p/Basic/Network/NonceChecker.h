#ifndef P2P_MSG_NONCECHECKER_H
#define P2P_MSG_NONCECHECKER_H

#include <bitset>

namespace p2p::Basic::Network {
    template<typename Nonce, size_t queueSize>
    class NonceChecker {
    public:
        bool checkAndAdd(Nonce nonce) {
            if (nonce > _maxNonce) {
                size_t diff = nonce - _maxNonce;
                _queue <<= diff;
                _queue.set(0);
                return true;
            } else {
                size_t pos = _maxNonce - nonce;
                if (pos >= queueSize || _queue.test(pos))
                    return false;
                _queue.set(pos);
                return true;
            }
        }

    private:
        std::bitset<queueSize> _queue;
        Nonce _maxNonce;
    };
}

#endif //P2P_MSG_NONCECHECKER_H
