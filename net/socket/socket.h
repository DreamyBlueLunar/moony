#pragma once

namespace moony {
class inetaddress;

class socket {
public:
    explicit socket(int sockfd):
        sockfd_(sockfd) {}
    ~socket();

    int fd() const {    return sockfd_; }
    void bind_address(const inetaddress& local_addr);
    void listen();
    int accept(inetaddress* peer_addr);

    void shutdown_write();
    void set_TCP_no_delay(bool on);
    void set_reuse_addr(bool on);
    void set_reuse_port(bool on);
    void set_keep_alive(bool on);

private:
    int sockfd_;
};   
}
