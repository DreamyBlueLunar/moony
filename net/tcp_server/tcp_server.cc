#include "tcp_server.h"
#include "../../base/logger/logger.h"

static moony::event_loop* check_loop_not_null(moony::event_loop* loop) {
    if (nullptr == loop) {
        LOG_FMT_FATAL("%s:%s:%d main loop is null",
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
        next_id_(1) {
    // 当有新用户连接时，会执行 tcp_server::new_connection() 回调
    // 对应到代码上就是 acceptor::handle_read()
    acceptor_->set_connection_callback(
                std::bind(&tcp_server::new_connection, 
                this, 
                std::placeholders::_1, 
                std::placeholders::_2));
}

void moony::tcp_server::set_thread_num(int num_threads) {
    thread_pool_->set_threads_nums(num_threads);
}

// 这个调用结束以后，就是 loop.loop()
void moony::tcp_server::start() {
    if (started_ ++ == 0) {
        thread_pool_->start(thread_init_callback_);
        loop_->run_in_loop(std::bind(&acceptor::listen, acceptor_.get()));
    }
}

void moony::tcp_server::new_connection(int sockfd, const moony::inetaddress& addr) {

}