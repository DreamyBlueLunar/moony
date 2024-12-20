#include "poller.h"
#include "epoll_poller.h"

moony::poller* moony::poller::new_default_poller(moony::event_loop* loop) {
    // add code
    // if (::getenv("LEEMUDUO_USE_POLL")) {
    //     return poll_poller(loop);
    // } else {
        return new epoll_poller(loop);
    // }

    // return nullptr;
}