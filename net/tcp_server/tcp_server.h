#pragma once

/**
 * 用户使用 muduo 网络库
 */

#include "../event_loop/event_loop.h"
#include "../acceptor/acceptor.h"
#include "../inetaddress/inetaddress.h"
#include "../../base/noncopyable.h"
#include "../../base/callbacks.h"
#include "../event_loop_thread_pool/event_loop_thread_pool.h"

#include <functional>
#include <unordered_map>
#include <atomic> // C++ 11

namespace moony {

class tcp_server : noncopyable {
public:
    using thread_init_callback = std::function<void(event_loop*)>;

    enum option {
        k_no_reuse_port,
        k_reuse_port
    };

    tcp_server(event_loop* loop, 
                const inetaddress& listen_addr, 
                const std::string name, 
                option op = k_no_reuse_port);
    ~tcp_server();

    const std::string& ip_port() const {  return ip_port_; }
    const std::string& name() const {  return name_; }
    event_loop* get_loop() const {    return loop_;   }

    void set_thread_init_callback(const thread_init_callback& cb) {
        thread_init_callback_ = std::move(cb);
    }
    void set_connection_callback(const connection_callback& cb) {
        connection_callback_ = std::move(cb);
    }
    void set_write_complete_callback(const write_complete_callback& cb) {
        write_complete_callback_ = std::move(cb);
    }
    void set_message_callback(const message_callback& cb) {
        message_callback_ = std::move(cb);
    }

    // 设置 sub_loop 个数
    void set_thread_num(int num_threads);
    // 开启服务器监听
    void start();

private:
    using connection_map = std::unordered_map<std::string, tcp_connection_ptr>;

    // not thread safe, but in loop
    void new_connection(int sockfd, const inetaddress& peer_addr);
    // thread safe
    void remove_connection(const tcp_connection_ptr& conn);
    // not thread safe, but in loop
    void remove_connection_in_loop(const tcp_connection_ptr& conn);
    
    event_loop* loop_; // base_loop，用户自定义的 loop

    const std::string ip_port_;
    const std::string name_;
    
    std::unique_ptr<acceptor> acceptor_; // 运行在 main_loop，任务是监听新连接事件
    
    std::shared_ptr<event_loop_thread_pool> thread_pool_; // one loop per thread

    connection_callback connection_callback_; // 有连接时执行的回调
    message_callback message_callback_; // 有消息时执行的回调
    write_complete_callback write_complete_callback_; // 写操作完成的回调

    thread_init_callback thread_init_callback_; // loop 线程初始化的回调
    std::atomic_int started_;

    int next_id_;
    connection_map connections_; // 保存所有的连接   
};


}