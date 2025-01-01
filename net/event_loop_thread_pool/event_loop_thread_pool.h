#pragma once

#include "../../base/noncopyable.h"

#include <functional>
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace moony {
class event_loop;
class event_loop_thread;

class event_loop_thread_pool : noncopyable {
public:
    using thread_init_callback = std::function<void(event_loop*)>;

    event_loop_thread_pool(event_loop* base_loop, const std::string& name);
    ~event_loop_thread_pool();

    void set_threads_nums(int num_threads) {    num_threads_ = num_threads; }
    void start(const thread_init_callback& cb = thread_init_callback());    

    event_loop* get_next_loop();
    std::vector<event_loop*> get_all_loops();

    bool started() const {  return started_;    }
    const std::string& name() const {   return name_;   }

private:
    event_loop* base_loop_; // event_loop loop; -> 用户手动创建出来的 loop
    bool started_;
    std::string name_;
    int num_threads_;
    int next_;
    std::vector<std::unique_ptr<event_loop_thread>> threads_;
    std::vector<event_loop*> loops_;
};
}