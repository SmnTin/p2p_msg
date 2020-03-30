#ifndef P2P_MSG_MESSENGER_MESSAGE_H
#define P2P_MSG_MESSENGER_MESSAGE_H

#include <ctime>
#include <string>

namespace p2p {
    namespace Messenger {

        struct Message {
            bool isSelf = true;
            time_t date = time(0);
            std::string content;
        };
    } //Messenger
} //p2p

#endif //P2P_MSG_MESSENGER_MESSAGE_H
