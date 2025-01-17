#include "inetaddress.h"

#include <strings.h>
#include <string.h>

moony::inetaddress::inetaddress(): moony::inetaddress(9080, "127.0.0.1") {}
    
moony::inetaddress::inetaddress(uint32_t port, std::string ip) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

moony::inetaddress::~inetaddress() {}

std::string moony::inetaddress::to_ip() const {
    char buffer[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buffer, sizeof(buffer));
    return buffer;
}

std::string moony::inetaddress::to_ip_port() const {
    char buffer[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buffer, sizeof(buffer));
    int len = strlen(buffer);
    uint32_t port = ntohs(addr_.sin_port);
    sprintf(buffer + len, ":%u", port);
    return buffer;
}

uint32_t moony::inetaddress::to_port() const {
    return ntohs(addr_.sin_port);
}