#include "channel.h"
#include "../../base/logger/logger.h"

#include <sys/epoll.h>

const int k_none_event_ = 0;
const int k_read_event_ = EPOLLIN | EPOLLPRI;
const int k_write_event_ = EPOLLOUT;

lee::channel::channel(event_loop* loop, int fd):
                        loop_(loop), fd_(fd),
                        events_(0), revents_(0),
                        idx_(-1), tied_(false) {}

lee::channel::~channel() {}

/**
 * 用一个弱指针绑定当前 channel 以监听其生命周期
 */
void lee::channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

/**
 * 从 event_loop 中删除当前 channel
 */
void lee::channel::remove() {
    // add code
    // loop_->remove_channel(this);
}

/**
 * 在 event_loop 的 poller 中更新 channel 绑定的事件类型
 */
void lee::channel::update() {
    // add code
    // loop_->update_channel(this);
}

void lee::channel::handle_event(time_stamp receieve_time) {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handle_event_with_guard(receieve_time);
        }
    } else {
        handle_event_with_guard(receieve_time);
    }
}

/**
 * 根据事件的类型调用对应的回调
 */
void lee::channel::handle_event_with_guard(time_stamp receive_time) {
    LOG_FMT_INFO("channel handle event revents_: %d\n", revents_);    

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (close_callback_) {
            close_callback_();
        }
    }

    if (revents_ & EPOLLERR) {
        if (error_callback_) {
            error_callback_();
        }
    }

    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (read_callback_) {
            read_callback_(receive_time);
        }
    }

    if (revents_ & EPOLLOUT) {
        if (write_callback_) {
            write_callback_();
        }
    }
}

