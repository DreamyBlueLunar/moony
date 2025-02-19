#include "channel.h"
#include "../../base/logger/logger.h"

#include <sys/epoll.h>

const int moony::channel::k_none_event_ = 0;
const int moony::channel::k_read_event_ = EPOLLIN | EPOLLPRI;
const int moony::channel::k_write_event_ = EPOLLOUT;

moony::channel::channel(moony::event_loop* loop, int fd):
                        loop_(loop), fd_(fd),
                        events_(0), revents_(0),
                        idx_(-1), tied_(false) {}

moony::channel::~channel() {}

/**
 * channel 调用的回调实际上是 tcp_connection 的成员
 * tcp_connection => channel
 * 那这里的 tie_ 指针监听的实际上是 tcp_connection 对象的生命周期
 * 只有在 tcp_connection 对象还在的时候才能调用对应的回调
 * 
 * 绑定 channel 对象和 tcp_connection 对象
 * @param
 * obj: 一个强智能指针，指向要绑定的 tcp_connection 对象
 */
void moony::channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

/**
 * 从 event_loop 中删除当前 channel
 */
void moony::channel::remove() {
    loop_->remove_channel(this);
}

/**
 * 在 event_loop 的 poller 中更新 channel 绑定的事件类型
 */
void moony::channel::update() {
    loop_->update_channel(this);
}

void moony::channel::handle_event(moony::time_stamp receieve_time) {
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
void moony::channel::handle_event_with_guard(moony::time_stamp receive_time) {
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

