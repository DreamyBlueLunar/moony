#pragma once

#include "../../base/noncopyable.h"
#include "../../base/callbacks.h"
#include "../inetaddress/inetaddress.h"
#include "../buffer/buffer.h"
#include "../../base/time_stamp/time_stamp.h"

#include <memory>
#include <string>
#include <atomic>

namespace moony {

class channel;
class event_loop;
class socket;

/**
 * std::enable_shared_from_this 和 shared_from_this 为类的成员函数提供了一种安全获取自身
 * shared_ptr 的方法，特别适用于需要在异步操作、回调函数中延长对象生命周期的场景。
 * 合理使用它们可以有效避免内存管理问题，提高程序的健壮性。
 * 
 * 注意：
 *   避免在多个基类中重复继承 std::enable_shared_from_this，以防止二义性和错误
 *   确保对象是通过 std::shared_ptr 创建和管理的，以便 shared_from_this 能正常工作。
 *   使用 std::weak_ptr 持有对观察者的引用，防止循环引用，同时在需要时使用 shared_from_this 获取自身的 shared_ptr
 * 
 * tcp_server -> acceptor -> 通过 accept 函数拿到 connfd 
 * -> tcp_connection 设置回调 -> channel -> poller -> channel 执行回调
 */
class tcp_connection : noncopyable, 
        public std::enable_shared_from_this<tcp_connection> {
public:
    tcp_connection(event_loop* loop, 
                    const std::string& name,
                    int fd, 
                    const inetaddress& local_addr, 
                    const inetaddress& peer_addr);
    ~tcp_connection();

    event_loop* get_loop() const {  return loop_;   }
    const std::string& name() const {   return name_;   }
    const inetaddress& local_addr() const { return local_addr_; }
    const inetaddress& peer_addr() const { return peer_addr_; }

    bool connected() const {    return state_ == k_connected;   }
    bool disconnected() const { return state_ == k_disconnected;    }

    // 发送数据
    void send(const void* message, int len);
    // 关闭连接
    void shutdown();

    void set_connection_callback(const connection_callback& cb) {
        connection_callback_ = std::move(cb);
    }

    void set_write_complete_callback(const write_complete_callback& cb) {
        write_complete_callback_ = std::move(cb);
    }

    void set_message_callback(const message_callback& cb) {
        message_callback_ = std::move(cb);
    }

    void set_high_water_mark_callback(const high_water_mark_callback& cb, size_t high_water_mark) {
        high_water_mark_callback_ = std::move(cb);
        high_water_mark_ = high_water_mark;
    }

    void set_close_callback(const close_callback& cb) {
        close_callback_ = std::move(cb);
    }

    // 连接建立
    void connection_established();
    // 连接销毁
    void connection_destroyed();

private:
    enum state_e {k_disconnected, k_connecting, k_connected, k_disconnecting};

    void set_state(state_e state) { state_ = state; }

    void handle_read(time_stamp receieve_time);
    void handle_write();
    void handle_close();
    void handle_error();

    void send_in_loop(const void* message, int len);
    void shutdown_in_loop();
    
    event_loop* loop_; // 这个显然是sub_loop
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    // 这里和 acceptor 是类似的
    // acceptor -> main_loop
    // tcp_connection -> sub_loop
    std::unique_ptr<socket> socket_;
    std::unique_ptr<channel> channel_;

    const inetaddress local_addr_;
    const inetaddress peer_addr_;

    connection_callback connection_callback_;
    message_callback message_callback_;
    write_complete_callback write_complete_callback_;
    close_callback close_callback_;
    high_water_mark_callback high_water_mark_callback_;
    size_t high_water_mark_;

    buffer input_buffer_;
    buffer output_buffer_;
};

}