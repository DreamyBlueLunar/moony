#include "event_loop.h"
#include "../../base/logger/logger.h"
#include "../../base/current_thread/current_thread.h"
#include "../poller/poller.h"
#include "../channel/channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

namespace {
/* 防止一个线程创建多个 event_loop */
__thread moony::event_loop* t_loop_in_this_thread = nullptr;

/* poller timeout */
const int k_poll_time_ms = 10000;

/**
 * 创建 wakeup_fd，用于唤醒 sub_reactor 处理新到来的 channel
 * @return 
 * - evtfd: 返回值是eventfd()的返回值，要赋给 wakeup_fd_
 */
int create_event_fd() {
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FMT_FATAL("eventfd error: %d\n", errno);
    }

    return evtfd;
}
}

/**
 * 默认构造函数，要给各成员赋值，还要给线程内的t_loop_in_this_thread赋值，
 * 以指明当前线程内已经创建了 event_loop 对象；
 * 还要设置 read_callback_ 回调，以支持 sub_reactor 之间的通信
 */
moony::event_loop::event_loop():
        looping_(false),
        quit_(false),
        calling_pending_functors_(false),
        thread_id_(current_thread::tid()),
        poller_(moony::poller::new_default_poller(this)),
        wakeup_fd_(create_event_fd()),
        wakeup_channel_(new moony::channel(this, wakeup_fd_)) {
    LOG_FMT_DEBUG("A event_loop %p created in this thread\n", this);
    if (t_loop_in_this_thread) {
        LOG_FMT_FATAL("Another event_loop %p exists in this thread %d\n", 
                        this, thread_id_);
    } else {
        t_loop_in_this_thread = this;
    }

    wakeup_channel_->set_read_callback(std::bind(&moony::event_loop::handle_read, this));
    wakeup_channel_->enable_reading();
}

/**
 * 析构函数，释放系统资源，置 t_loop_in_this_thread 为空指针
 */
moony::event_loop::~event_loop() {
    wakeup_channel_->disable_all();
    wakeup_channel_->remove();
    ::close(wakeup_fd_);
    t_loop_in_this_thread = nullptr;
}

/**
 * 用户在最后的 main() 中调用的接口，启动事件循环。由 poller::poll_wait() 驱动
 * event_loop 类中最核心的逻辑
 */
void moony::event_loop::loop() {
    looping_ = true;
    quit_ = false;

    LOG_FMT_INFO("event_loop::loop() %p starts looping\n", this);

    while (!quit_) {
        active_channels_.clear();
        
        // 监听两类 fd，client_fd 和 wakeup_fd
        poller_return_time_ = poller_->poll_wait(k_poll_time_ms, &active_channels_);
    
        for (moony::channel* chann : active_channels_) {
            chann->handle_event(poller_return_time_);
        }

        // 由其他线程请求调用的用户任务，其实就是 another_loop->queue_in_loop() 为
        // 当前 event_loop 请求调用的任务，放在了 pending_functors_ 中
        do_pending_functions();
    }

    LOG_FMT_INFO("event_loop::loop() %p ends looping\n", this);
    looping_ = false;
}

/**
 * 退出事件循环，如果当前线程不是 event_loop 所在的线程，就唤醒对应的线程。
 * 所谓唤醒对应线程，其实就是向对应的 wakeup_fd_（eventfd） 中随便写点数据，个人认为这是一个很有意思的实现手法。
 */
void moony::event_loop::quit() {
    quit_ = true;

    if (!in_loop_thread()) {
        wakeup();
    }
}

/**
 * 向事件循环中添加想要执行的回调，如果在当前 loop 线程，执行回调，否则添加到队列中，唤醒 loop 所在线程，执行回调。
 * @prama:
 * - cb: 想要处理的回调
 */
void moony::event_loop::run_in_loop(functor cb) {
    if (in_loop_thread()) {
        cb();
    } else {
        queue_in_loop(std::move(cb));
    }
}

/**
 * 添加到队列中，唤醒 event_loop 所在线程，执行回调
 * @prama:
 * - cb: 想要处理的回调
 */
void moony::event_loop::queue_in_loop(functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pending_functors_.emplace_back(cb);
    }

    // 如果不在当前 loop 线程或者当前 loop 线程正在处理回调时来了新的回调，就唤醒对应 loop 线程
    if (!in_loop_thread() || calling_pending_functors_) {
        wakeup();
    }
}

/**
 * 处理 wakeup_fd_ 上到来的数据，其实主要作用是通过接收这点儿数据来唤醒线程
 */
void moony::event_loop::handle_read() {
  uint64_t one = 1;
  ssize_t n = read(wakeup_fd_, &one, sizeof(uint64_t));
  if (n != sizeof(uint64_t)) {
    LOG_FMT_ERROR("event_loop::handle_read() reads %ld bytes instead of 8\n", n);
  }
}

/** 
 * 用来唤醒 loop 所在的线程，向 wakeup_fd_ 写一个数据，
 * wakeup_channel 就发生一个读事件，对应的线程就会被唤醒
 */
void moony::event_loop::wakeup(){
    uint64_t one = 1;
    ssize_t n = write(wakeup_fd_, &one, sizeof(uint64_t));
    if (n != sizeof(uint64_t)) {
        LOG_FMT_ERROR("event_loop::wakeup() writes %lu bytes instead of 8\n", n);
    }    
}

/**
 * channel::update 调用的接口
 * @prama:
 * - chann: 要更新状态的 channel 指针
 */
void moony::event_loop::update_channel(channel* chann) {
    poller_->update_channel(chann);
}

/**
 * channel::remove 调用的接口
 * @prama:
 * - chann: 要从 poller 中注销状态的 channel 指针
 */
void moony::event_loop::remove_channel(channel* chann) {
    poller_->remove_channel(chann);
}

/**
 * 检查 poller 中是否存在 *chann
 * @prama:
 * - chann: 查找的 channel 指针
 */
bool moony::event_loop::has_channel(channel* chann) {
    return poller_->has_channel(chann);
}

/**
 * 处理回调操作，这一步执行的回调操作通常是由其它线程触发的
 */
void moony::event_loop::do_pending_functions() {
    std::vector<functor> functors;
    calling_pending_functors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    } 

    for (const functor& func : functors) {
        func(); // 执行当前 loop 需要执行的回调操作
    }

    calling_pending_functors_ = false;
}
