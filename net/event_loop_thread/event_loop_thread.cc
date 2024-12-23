#include "event_loop_thread.h"
#include "../event_loop/event_loop.h"


moony::event_loop_thread::event_loop_thread(
        const thread_init_callback& cb, 
        const std::string& name):
        loop_(nullptr),
        exiting_(false),
        thread_(std::bind(&event_loop_thread::thread_func, this)),
        mutex_(),
        cond_(),
        callback_(cb) {}

moony::event_loop_thread::~event_loop_thread() {
    exiting_ = true;
    if (nullptr != loop_) {
        loop_->quit();
        thread_.join();
    }
}

moony::event_loop* moony::event_loop_thread::start_loop() {
    thread_.start(); // 启动底层的新线程
    event_loop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (nullptr == loop) {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

// 在新线程中执行的函数
void moony::event_loop_thread::thread_func() {
    event_loop loop; // 上来就在新线程里创建一个 event_loop，one loop per thread

    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}