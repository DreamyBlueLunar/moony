#include "base_thread.h"
#include "../current_thread/current_thread.h"

#include <semaphore.h>

std::atomic_int moony::base_thread::num_created_ = 0;


explicit moony::base_thread::base_thread(thread_func func, 
        const std::string& name):
        started_(false),
        joined_(false),
        name_(name),
        func_(std::move(func)),
        tid_(0) {
    set_default_name();
}

moony::base_thread::~base_thread() {
    if (started_ && !joined_) {
        thread_->detach();
    }
}

// 一个 base_thread 对象就是记录新线程的信息
void moony::base_thread::start() {
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = current_thread::tid();
        sem_post(&sem);
        func_();
    }));

    sem_wait(&sem);
}

void moony::base_thread::join() {
    joined_ = true;
    thread_->join();
}

void moony::base_thread::set_default_name() {
    int num = ++ num_created_;

    if (name_.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "thread %d", num);
        name_ = buf;
    }
}