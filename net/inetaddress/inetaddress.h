#pragma once

#include "../../base/copyable.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

namespace lee {
/* 封装 socket 地址 */
class inetaddress : public lee::copyable {
public:
    explicit inetaddress();
    explicit inetaddress(uint32_t port, std::string ip);
    explicit inetaddress(const sockaddr_in& addr): addr_(addr) {}
    ~inetaddress();

    std::string to_ip() const;
    std::string to_ip_port() const;
    uint32_t to_port() const;
    const sockaddr* get_sock_addr() {   return (const sockaddr*)&addr_;  }

private:
    sockaddr_in addr_;
};
}