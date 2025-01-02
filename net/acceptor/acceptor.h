#pragma once

#include "../../base/noncopyable.h"
#include "../socket/socket.h"
#include "../channel/channel.h"

#include <functional>

namespace moony {
class event_loop;
class inetaddress;

class acceptor : noncopyable {
public:
    using new_connection_callback = std::function<void(int, const inetaddress&)>;

    acceptor(event_loop* loop, const inetaddress& listen_addr, bool reuseport);
    ~acceptor();

    void set_connection_callback(const new_connection_callback& cb) {
        new_connection_callback_ = std::move(cb);
    }

    bool listenning() { return listenning_; }
    void listen();

private:
    void handle_read();

    event_loop* loop_;
    socket accept_socket_;
    channel accept_channel_;
    new_connection_callback new_connection_callback_;
    bool listenning_;
};
}