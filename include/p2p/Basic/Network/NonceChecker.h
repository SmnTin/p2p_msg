#ifndef P2P_MSG_NONCECHECKER_H
#define P2P_MSG_NONCECHECKER_H

#include <bitset>

namespace p2p::Basic::Network {
    template<typename Nonce, size_t slidingWindowWidth>
    class NonceChecker {
    public:
        bool check(Nonce nonce) {
            return isSurelyNewNonce(nonce) || isNotDuplicatedNonce(nonce);
        }

        void update(Nonce nonce) {
            if (isSurelyNewNonce(nonce))
                updateWithCompletelyNewNonce(nonce);
            else if (isNotDuplicatedNonce(nonce))
                updateWithNotDuplicatedNonce(nonce);
        }

        bool checkAndUpdate(Nonce nonce) {
            if (check(nonce)) {
                update(nonce);
                return true;
            }
            return false;
        }

    private:
        bool isSurelyNewNonce(Nonce nonce) {
            return nonce > _maxNonce;
        }

        bool isNotDuplicatedNonce(Nonce nonce) {
            size_t pos = _maxNonce - nonce;
            return pos < slidingWindowWidth && !_slidingWindow.test(pos);
        }

        void updateWithCompletelyNewNonce(Nonce nonce) {
            size_t diff = nonce - _maxNonce;
            shiftTheWindow(diff);
            _maxNonce = nonce;
            setNonceAsUsedInTheWindow(nonce);
        }

        void shiftTheWindow(size_t diff) {
            _slidingWindow <<= diff;
        }

        void updateWithNotDuplicatedNonce(Nonce nonce) {
            setNonceAsUsedInTheWindow(nonce);
        }

        void setNonceAsUsedInTheWindow(Nonce nonce) {
            size_t pos = _maxNonce - nonce;
            _slidingWindow.set(pos);
        }

        std::bitset<slidingWindowWidth> _slidingWindow;
        Nonce _maxNonce = 0;
    };
}

#endif //P2P_MSG_NONCECHECKER_H
