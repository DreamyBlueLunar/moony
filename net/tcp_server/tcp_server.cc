#include "tcp_server.h"
#include "../../base/logger/logger.h"
#include "../tcp_connection/tcp_connection.h"

#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

static moony::event_loop* check_loop_not_null(moony::event_loop* loop) {
    if (nullptr == loop) {
        LOG_FMT_FATAL("%s:%s:%d main loop is null\n",
                        __FILE__, __FUNCTION__, __LINE__);        
    }

    return loop;
}

moony::tcp_server::tcp_server(moony::event_loop* loop, 
        const moony::inetaddress& listen_addr, 
        const std::string name, 
        option op):
        loop_(check_loop_not_null(loop)),
        ip_port_(listen_addr.to_ip_port()),
        name_(name),
        acceptor_(new moony::acceptor(loop, listen_addr, op == k_reuse_port)),
        thread_pool_(new moony::event_loop_thread_pool(loop, name)),
        connection_callback_(),
        message_callback_(),
        next_id_(1),
        started_(0) {
    // 当有新用户连接时，会执行 tcp_server::new_connection() 回调
    // 对应到代码上就是 acceptor::handle_read()
    acceptor_->set_connection_callback(
                std::bind(&moony::tcp_server::new_connection, 
                this, 
                std::placeholders::_1, 
                std::placeholders::_2));
}

moony::tcp_server::~tcp_server() {
    for (auto& item : connections_) {
        // 这个局部的智能指针对象出了右括号就可以自动释放 new 出来的 tcp_connection 对象资源 
        moony::tcp_connection_ptr conn(item.second);
        item.second.reset();

        conn->get_loop()->run_in_loop(
            std::bind(&moony::tcp_connection::connection_destroyed, conn));
    }
}

void moony::tcp_server::set_thread_num(int num_threads) {
    thread_pool_->set_threads_nums(num_threads);
}

// 这个调用结束以后，就是 loop.loop()
void moony::tcp_server::start() {
    if (started_ ++ == 0) {
        thread_pool_->start(thread_init_callback_);
        loop_->run_in_loop(std::bind(&moony::acceptor::listen, acceptor_.get()));
    }
}

// 有一个新的客户端连接，acceptor 会执行这个回调操作
void moony::tcp_server::new_connection(int sockfd, 
        const moony::inetaddress& peer_addr) {
    // 轮询算法拿到某个 sub_loop 来管理 channel
    moony::event_loop* io_loop = thread_pool_->get_next_loop();
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_id_);
    next_id_ ++;
    std::string conn_name = name_ + buf;

    LOG_FMT_INFO(
        "tcp_server::new_connection [%s] - new connection [%s] from %s\n",
        name_.c_str(),
        conn_name.c_str(),
        peer_addr.to_ip_port().c_str());

    // 通过 sockfd 获取本机 ip 和 port
    sockaddr_in local;
    ::bzero(&local, sizeof(local));
    socklen_t len = sizeof(local);
    if (::getsockname(sockfd, (sockaddr*)&local, &len) < 0) {
        LOG_FMT_ERROR("%s:%s:%d failed to get local address\n",
                __FILE__, __FUNCTION__, __LINE__);
    }
    moony::inetaddress local_addr(local);

    // 根据连接成功的 sockfd 创建 tcp_connection 对象
    moony::tcp_connection_ptr conn(
        new moony::tcp_connection(io_loop, conn_name, 
                            sockfd, local_addr, peer_addr));

    connections_[conn_name] = conn;

    // 下面的回调都是用户设置给 tcp_server => tcp_connection => channel => poller => notify channel 调用回调
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    // 设置了如何关闭连接的回调
    conn->set_close_callback(
        std::bind(&moony::tcp_server::remove_connection,
                    this, 
                    std::placeholders::_1));

    // 直接调用 tcp_connection::connection_established
    io_loop->run_in_loop(
        std::bind(&moony::tcp_connection::connection_established, conn));
}

void moony::tcp_server::remove_connection(
        const moony::tcp_connection_ptr& conn) {
    loop_->run_in_loop(
        std::bind(&moony::tcp_server::remove_connection_in_loop,
                    this,
                    conn));
}

void moony::tcp_server::remove_connection_in_loop(
        const moony::tcp_connection_ptr& conn) {
    LOG_FMT_INFO(
        "tcp_server::remove_connection_in_loop [%s] - connection %s\n",
        name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());
    moony::event_loop* io_loop = conn->get_loop();
    io_loop->queue_in_loop(
        std::bind(&moony::tcp_connection::connection_destroyed, conn));
}