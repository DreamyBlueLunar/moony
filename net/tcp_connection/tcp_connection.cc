#include "tcp_connection.h"
#include "../../base/logger/logger.h"
#include "../socket/socket.h"
#include "../channel/channel.h"

#include <functional>
#include <errno.h>
#include <memory>

static moony::event_loop* check_loop_not_null(moony::event_loop* loop) {
    if (nullptr == loop) {
        LOG_FMT_FATAL("%s:%s:%d tcp_connection loop is null",
                        __FILE__, __FUNCTION__, __LINE__);        
    }

    return loop;
}

moony::tcp_connection::tcp_connection(moony::event_loop* loop, 
        const std::string& name, int fd, 
        const moony::inetaddress& local_addr, 
        const moony::inetaddress& peer_addr):
        loop_(check_loop_not_null(loop)),
        name_(name),
        state_(k_connecting),
        reading_(true),
        socket_(new socket(fd)),
        channel_(new channel(loop, fd)),
        local_addr_(local_addr),
        peer_addr_(peer_addr),
        high_water_mark_(64 * 1024 * 1024) {
    channel_->set_read_callback(
        std::bind(&moony::tcp_connection::handle_read, this, std::placeholders::_1));
    channel_->set_write_callback(
        std::bind(&moony::tcp_connection::handle_write, this));
    channel_->set_close_callback(
        std::bind(&moony::tcp_connection::handle_close, this));    
    channel_->set_error_callback(
        std::bind(&moony::tcp_connection::handle_error, this));
    LOG_FMT_INFO("tcp_connection ctor[%s] at fd = %d\n", name_.c_str(), fd);
    socket_->set_keep_alive(true);
}

moony::tcp_connection::~tcp_connection() {
    LOG_FMT_INFO("tcp_connection dtor[%s] at fd = %d, state = %d\n",
        name_.c_str(), channel_->fd(), state_);
}

void moony::tcp_connection::handle_read(moony::time_stamp receieve_time) {
    int save_errno = 0;
    ssize_t ret = input_buffer_.read_fd(channel_->fd(), &save_errno);
    if (ret > 0) {
        message_callback_(shared_from_this(), &input_buffer_, receieve_time);
    } else if (0 == ret) {
        handle_close();
    } else {
        errno = save_errno;
        LOG_FMT_ERROR("tcp_connection::handle_read\n");
        handle_error();
    }
}

void moony::tcp_connection::handle_write() {
    int save_errno = 0;
    ssize_t ret = output_buffer_.write_fd(channel_->fd(), &save_errno);

}

void moony::tcp_connection::handle_close() {

}

void moony::tcp_connection::handle_error() {

}

