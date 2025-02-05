#include "socket.h"
#include "../inetaddress/inetaddress.h"
#include "../../base/logger/logger.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>

moony::socket::~socket() {
    ::close(sockfd_);
}

void moony::socket::bind_address(const inetaddress& local_addr) {
    int ret = bind(sockfd_, local_addr.get_sock_addr(), sizeof(sockaddr_in));
    if (ret < 0) {
        LOG_FMT_FATAL("bind sockfd_: %d failed\n", sockfd_);        
    }
}

void moony::socket::listen() {
    int ret = ::listen(sockfd_, 1024);
    if (ret < 0) {
        LOG_FMT_FATAL("listen sockfd_: %d failed\n", sockfd_);        
    }
}

int moony::socket::accept(inetaddress* peer_addr) {
    sockaddr_in addr;
    bzero(&addr, sizeof(sockaddr_in));
    socklen_t len = sizeof(sockaddr);

    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        peer_addr->set_sock_addr(addr);
    }

    return connfd;
}

void moony::socket::shutdown_write() {
    int ret = shutdown(sockfd_, SHUT_WR);
    if (ret < 0) {
        LOG_FMT_ERROR("shutdown_write error\n");
    }
}

void moony::socket::set_TCP_no_delay(bool on) {
    int opt_val = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof(opt_val));
}

void moony::socket::set_reuse_addr(bool on) {
    int opt_val = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
}

void moony::socket::set_reuse_port(bool on) {
    int opt_val = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val));
}

void moony::socket::set_keep_alive(bool on) {
    int opt_val = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt_val, sizeof(opt_val));
}