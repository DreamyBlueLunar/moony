#pragma once

#include "../../base/noncopyable.h"
#include "../../base/time_stamp/time_stamp.h"

#include <functional>
#include <memory>

namespace lee {
class event_loop;

/**
 * event_loop、channel、poller之间的关系？
 * A: 一个线程对应一个 event_loop，一个 event_loop 由一个 poller 实现，
 * 而这个线程可以是主线程，可以是工作线程，显然一个线程处理的事件不止一个，
 * 所以可以对应多个 channel。这里的channel显然可以看作是 reactor 感兴趣的事件的封装。
 * 这三个东西组合起来对应 reactor 模型中的 Demultiplexer。
 * 
 * channel 理解成通道，其中封装了 sockfd 和其感兴趣的事件，如EPOLLIN、EPOLLOUT
 * 还封装了一个 poller 用于监听 fd 上发生的事件
 */
class channel : lee::noncopyable {
public:
    using event_callback = std::function<void()>;
    using read_event_callback = std::function<void(time_stamp)>;

    channel(event_loop* loop, int fd);
    ~channel();

    void tie(const std::shared_ptr<void>&);

    void handle_event(time_stamp receieve_time);
    void set_read_callback(read_event_callback& cb) {    read_callback_ = std::move(cb);    }
    void set_write_callback(event_callback& cb) {   write_callback_ = std::move(cb);    }
    void set_close_callback(event_callback& cb) {   close_callback_ = std::move(cb);    }
    void set_error_callback(event_callback& cb) {   error_callback_ = std::move(cb);    }

    int fd() const {    return fd_; }
    int events() const {    return events_; }
    void set_revents(int revt) { revents_ = revt;    }

    // 设置fd相应的事件状态
    void enable_reading() { events_ |= k_read_event_; update(); }
    void disable_reading() { events_ &= ~k_read_event_; update(); }
    void enable_writing() { events_ |= k_write_event_; update();    }
    void disable_writing() { events_ &= ~k_write_event_; update();    }
    void disable_all() { events_ = k_none_event_; update();  }

    // 返回fd相应的事件状态
    bool is_reading() { return events_ & k_read_event_; }
    bool is_wrting() { return events_ & k_write_event_; }
    bool is_none_event() { return events_ & k_none_event_;  }

    int index() { return idx_;  }
    void set_index(int idx) { idx_ = idx;   }

    // one loop per thread
    event_loop* owner_loop() {  return loop_;   }
    void remove();

    // 日志系统相关的封装暂时忽略掉
private:
    void update();
    void handle_event_with_guard(time_stamp receive_time);

    static const int k_none_event_;
    static const int k_read_event_;
    static const int k_write_event_;

    event_loop *loop_;
    const int fd_; // poller 监听的对象
    int events_;   // 注册fd感兴趣的事件
    int revents_;  // 返回具体发生的事件
    int idx_;

    std::weak_ptr<void> tie_; // 防止手动调用remove之后还在使用这个channel，多线程显然需要考虑这一点
    bool tied_;

    // 因为channel通道里可以获知revents，所以它调用事件的回调函数
    read_event_callback read_callback_;
    event_callback write_callback_;
    event_callback close_callback_;
    event_callback error_callback_;
};
}