#include "acceptor.h"
#include "../../base/logger/logger.h"
#include "../inetaddress/inetaddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

static int create_non_blocking_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_FMT_FATAL("%s:%s:%d create sockfd error: %d\n", 
                        __FILE__, __FUNCTION__, __LINE__, errno);        
    }

    return sockfd;
}

moony::acceptor::acceptor(moony::event_loop* loop, 
        const moony::inetaddress& listen_addr, bool reuseport):
        loop_(loop),
        accept_socket_(create_non_blocking_socket()),
        accept_channel_(loop, accept_socket_.fd()),
        listenning_(false) {
    accept_socket_.set_reuse_addr(true);
    accept_socket_.set_reuse_port(true);
    accept_socket_.bind_address(listen_addr);
    accept_channel_.set_read_callback(
        std::bind(&moony::acceptor::handle_read, this));
}

moony::acceptor::~acceptor() {
    accept_channel_.disable_all();
    accept_channel_.remove();
}

void moony::acceptor::listen() {
    listenning_ = true;
    accept_socket_.listen();
    accept_channel_.enable_reading(); // accept_channel_ 注册到 poller
}

void moony::acceptor::handle_read() {
    moony::inetaddress peer_addr;
    int connfd = accept_socket_.accept(&peer_addr);
    if (connfd >= 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connfd, peer_addr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_FMT_ERROR("%s:%s:%d accept error: %d\n", 
                        __FILE__, __FUNCTION__, __LINE__, errno);   
        if (errno == EMFILE) {
             LOG_FMT_ERROR("%s:%s:%d fd reached limit\n", 
                        __FILE__, __FUNCTION__, __LINE__);   
        }
    }
}