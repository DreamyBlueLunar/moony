#pragma once

#include "../../base/noncopyable.h"
#include "../../base/base_thread/base_thread.h"

#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

namespace moony {
class event_loop;

class event_loop_thread : noncopyable {
public:
    using thread_init_callback = std::function<void(event_loop*)>;

    event_loop_thread(const thread_init_callback& cb = thread_init_callback(),
                        const std::string& name = std::string());
    ~event_loop_thread();

    event_loop* start_loop();

private:
    void thread_func();

    event_loop* loop_;
    bool exiting_;
    base_thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    thread_init_callback callback_;
};
}