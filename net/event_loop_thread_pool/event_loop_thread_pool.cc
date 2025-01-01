#include "event_loop_thread_pool.h"
#include "../event_loop_thread/event_loop_thread.h"

#include <memory>
#include <vector>

moony::event_loop_thread_pool::
event_loop_thread_pool(event_loop* base_loop, const std::string& name):
        base_loop_(base_loop),
        name_(name),
        started_(false),
        num_threads_(0),
        next_(0)
{}

// 不 delete loop，这是个栈对象
moony::event_loop_thread_pool::~event_loop_thread_pool() {}

void moony::event_loop_thread_pool::start(const thread_init_callback& cb) {
    started_ = true;

    for (int i = 0; i < num_threads_; i ++) {
        char buf[name_.size() + 32] = {'\0'};
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        event_loop_thread* t = new event_loop_thread();
        threads_.push_back(std::unique_ptr<event_loop_thread>(t));
        loops_.push_back(t->start_loop()); // 底层创建线程，绑定一个新的 event_loop，并返回该 loop 的地址
    }

    if (num_threads_ == 0 && cb) {
        cb(base_loop_);
    }
}

moony::event_loop* moony::event_loop_thread_pool::get_next_loop() {
    event_loop* loop = base_loop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        next_ ++;
        if (next_ >= loops_.size()) {
            next_ = 0;
        }
    }

    return loop;
}

std::vector<moony::event_loop*> moony::event_loop_thread_pool::get_all_loops() {
    if (loops_.empty()) {
        return std::vector<event_loop*>(1, base_loop_);
    } else {
        return loops_;
    }
}