#include <moony/net/tcp_server/tcp_server.h>
#include <moony/base/logger/logger.h>

#include <string>
#include <functional>

class echo_server {
public:
    echo_server(moony::event_loop* loop, 
            const moony::inetaddress& addr,
            const std::string& name):
                server_(loop, addr, name), loop_(loop) {
        // set callbacks
        std::bind(&echo_server::on_connection, this, std::placeholders::_1);
        std::bind(&echo_server::on_message, this, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        // set thread nums
        server_.set_thread_num(3);
    }

    void start() {
        server_.start();
    }

    ~echo_server() {}
private:
    void on_connection(const moony::tcp_connection_ptr& conn) {
        if (conn->connected()) {
            LOG_FMT_INFO("Connection UP: %s\n", conn->peer_addr().to_ip_port().c_str());
        } else {
            LOG_FMT_INFO("Connection DOWN: %s\n", conn->peer_addr().to_ip_port().c_str());
        }
    }

    void on_message(const moony::tcp_connection_ptr& conn, 
            moony::buffer* buf,
            moony::time_stamp time) {
        std::string msg = buf->retrieve_all_as_string();
        conn->send(msg);
        conn->shutdown(); // 关闭写端 EPOLLHUP close_call_back_ 服务器主动关闭连接
    }

    moony::event_loop* loop_;
    moony::tcp_server server_;
};

int main(void) {
    moony::event_loop loop;
    moony::inetaddress addr;
    
    echo_server server(&loop, addr, "echo_server_01");
    
    server.start();
    loop.loop();

    return 0;
}