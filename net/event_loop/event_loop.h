#pragma once

#include "../../base/current_thread/current_thread.h"
#include "../../base/time_stamp/time_stamp.h"
#include "../../base/noncopyable.h"

#include <memory>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>

namespace moony {
class channel;
class poller;

/**
 * event_loop、channel、poller之间的关系？
 * A: 一个线程对应一个 event_loop，一个 event_loop 由一个 poller 实现，
 * 而这个线程可以是主线程，可以是工作线程，显然一个线程处理的事件不止一个，
 * 所以可以对应多个 channel。这里的channel显然可以看作是 reactor 感兴趣的事件的封装。
 * 
 * event_loop 对应 Reactor 模式中的 Reactor 模块，里面包含一个 I/O复用接口以及若干个事件
 */
class event_loop : noncopyable {
public:
    using functor = std::function<void()>;

    event_loop();
    ~event_loop();

    void loop(); // 开启事件循环，这个是交给用户调用的接口
    void quit(); // 退出事件循环

    time_stamp poller_return_time() const { return poller_return_time_; } // 返回poller发生事件的时间点

    void run_in_loop(functor cb); // 在当前 loop 中执行 cb
    void queue_in_loop(functor cb); // 将 cb 放入队列，唤醒 loop 所在的线程，执行 cb

    void wakeup(); // 唤醒 loop 所在的线程

    void update_channel(channel* chann); // channel::update 调用的接口，里面调用 poller::update_channel()
    void remove_channel(channel* chann); // channel::remove 调用的接口，里面调用 poller::remove_channel()
    bool has_channel(channel* chann); 

    bool in_loop_thread() { return thread_id_ == current_thread::tid(); } // 判断 event_loop 对象是否在自己的线程里面

private:
    void handle_read(); // 执行唤醒相关的逻辑
    void do_pending_functions(); // 执行回调

    using channel_list = std::vector<channel*>;

    std::atomic_bool looping_; // 原子操作，判断事件循环是否继续，通过 CAS 实现
    std::atomic_bool quit_;    // 原子操作，判断事件循环是否停止

    const pid_t thread_id_;    // 记录当前事件循环所在的线程 ID

    time_stamp poller_return_time_; // poller 返回发生事件的时间点

    std::unique_ptr<poller> poller_; // event_loop 维护的 poller

    int wakeup_fd_; // 当 main_loop 获取到一个新用户的 channel，通过 wakeup_fd 唤醒 sub_loop 处理
    std::unique_ptr<channel> wakeup_channel_; // 唤醒的 channel

    channel_list active_channels_;

    std::atomic_bool calling_pending_functors_; // 标记当前是否有需要执行的回调
    std::vector<functor> pending_functors_;      // 存储 loop 需要执行的所有回调
    std::mutex mutex_; // 互斥锁，确保线程安全
};

}
