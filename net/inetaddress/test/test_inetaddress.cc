#include "../inetaddress.h"

#include <gtest/gtest.h>

TEST(INETADDRESS, TEST_DEFAULT_CONSTRUCTOR) {
    lee::inetaddress addr;
    std::string ip = addr.to_ip();
    std::string ip_port = addr.to_ip_port();
    uint32_t port = addr.to_port();

    EXPECT_EQ("127.0.0.1:9080", ip_port);
    EXPECT_EQ("127.0.0.1", ip);
    EXPECT_EQ(9080, port);
}

TEST(INETADDRESS, TEST_CONSTRUCTOR) {
    lee::inetaddress addr(9081, "192.168.111.138");
    std::string ip = addr.to_ip();
    std::string ip_port = addr.to_ip_port();
    uint32_t port = addr.to_port();

    EXPECT_EQ("192.168.111.138:9081", ip_port);
    EXPECT_EQ("192.168.111.138", ip);
    EXPECT_EQ(9081, port);
}

TEST(INETADDRESS, TEST_COPY_CONSTRUCTOR) {
    lee::inetaddress addr(9081, "192.168.111.138");
    lee::inetaddress addr_cp(addr);

    std::string ip = addr_cp.to_ip();
    std::string ip_port = addr_cp.to_ip_port();
    uint32_t port = addr_cp.to_port();

    EXPECT_EQ("192.168.111.138:9081", ip_port);
    EXPECT_EQ("192.168.111.138", ip);
    EXPECT_EQ(9081, port);
}