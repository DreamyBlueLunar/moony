#include "poller.h"

lee::poller* lee::poller::new_default_poller(event_loop* loop) {
    // add code
    // if (::getenv("LEEMUDUO_USE_POLL")) {
    //     return poll_poller(loop);
    // } else {
    //     return epoll_poller(loop);
    // }

    return nullptr;
}