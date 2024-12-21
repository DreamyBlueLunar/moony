#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <string>
#include <atomic>

namespace moony {
class base_thread {
public:
    using thread_func = std::function<void()>;

    explicit base_thread(thread_func func, 
                            const std::string& name = std::string());
    ~base_thread();

    void start();
    void join();

    bool started() const {  return started_;    }
    bool joined() const {   return joined_; }
    pid_t tid() const { return tid_;    }
    const std::string& name() { return name_;   }
    static int num_created() {  return num_created_;    }

private:
    void set_default_name();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    thread_func func_;
    std::string name_;
    static std::atomic_int num_created_;
};

}