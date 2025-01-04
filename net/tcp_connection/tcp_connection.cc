#include "tcp_connection.h"
#include "../../base/logger/logger.h"
#include "../socket/socket.h"
#include "../channel/channel.h"
#include "../event_loop/event_loop.h"

#include <functional>
#include <errno.h>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string>
#include <unistd.h>

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
        name_.c_str(), channel_->fd(), static_cast<int>(state_));
}

void moony::tcp_connection::send(const std::string& buf) {
    if (k_connected == state_) {
        if (loop_->in_loop_thread()) {
            send_in_loop(buf.c_str(), buf.size());
        } else {
            loop_->run_in_loop(
                std::bind(&tcp_connection::send_in_loop,
                            this,
                            buf.c_str(),
                            buf.size()));
        }
    }
}

/**
 * 发送数据，由于应用写得快，内核写得慢，需要把待发送数据放入缓冲区并设置水位回调
 * @param
 * data: 发送数据的起始地址
 * len: 发送数据的长度
 */
void moony::tcp_connection::send_in_loop(const void* data, int len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fault_error = false;

    if (k_disconnected == state_) {
        LOG_FMT_ERROR("disconnected, give up writing\n");
        return ;
    }
    // channel_ 第一次发送数据
    if (!channel_->is_writing() && 0 == output_buffer_.readable_bytes()) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            // 既然在这里数据全部发送完了，自然也就不用注册 EPOLLOUT 事件，触发 handle_write了
            if (0 == remaining && write_complete_callback_) {
                loop_->queue_in_loop(
                    std::bind(write_complete_callback_, shared_from_this()));
            }
        } else { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_FMT_ERROR("tcp_connection::send_in_loop\n");
                if (errno == EPIPE || errno == ECONNRESET) { // SIGPIPE RESET
                    fault_error = true;
                }
            }
        }
    }

    // 当前这一次 write 没有把数据全部发送出去，就应该向底层的 poller 注册一个
    // EPOLLOUT 事件，当 poller 监听到对应的 TCP 缓冲区中有可写空间的时候，就通知
    // 对应的 channel 调用 (channel::write_callback) => handle_read 回调来写入数据
    if (!fault_error && remaining > 0) {
        // 目前 output_buffer_ 中要发送的数据长度
        size_t oldlen = output_buffer_.readable_bytes();
        if (oldlen + remaining >= high_water_mark_ &&
                oldlen < high_water_mark_ &&
                high_water_mark_callback_) {
            loop_->queue_in_loop(
                std::bind(high_water_mark_callback_,
                            shared_from_this(),
                            oldlen + remaining));
        }
        output_buffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->is_writing()) { // 注册 EPOLLOUT 事件     
            channel_->enable_writing();
        }
    }
}


void moony::tcp_connection::handle_read(moony::time_stamp receieve_time) {
    int save_errno = 0;
    ssize_t ret = input_buffer_.read_fd(channel_->fd(), &save_errno);
    if (ret > 0) { // 已经建立连接的用户，有可读事件发生了，调用用户传入的回调操作 on_message
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
    if (channel_->is_writing()) {
        int save_errno = 0;
        ssize_t ret = output_buffer_.write_fd(channel_->fd(), &save_errno);
        
        if (ret > 0) {
            output_buffer_.retrieve(ret);
            if (0 == output_buffer_.readable_bytes()) { // 如果数据发送完了
                channel_->disable_writing();
                if (write_complete_callback_) {
                    loop_->queue_in_loop( // 唤醒 loop_ 对应的线程，执行回调
                        std::bind(write_complete_callback_, shared_from_this()));
                }

                if (k_disconnecting == state_) {
                    shutdown_in_loop();
                }
            }
        } else {
            LOG_FMT_ERROR("tcp_connection::handle_write\n");
        }
    } else {
        LOG_FMT_ERROR("tcp_connection fd = %d is down, no more writing\n",
                        channel_->fd());
    }
}

void moony::tcp_connection::handle_close() {
    LOG_FMT_INFO("fd = %d, state = %d\n",
                channel_->fd(), static_cast<int>(state_));
    set_state(k_disconnected);
    channel_->disable_all();

    tcp_connection_ptr conn(shared_from_this());
    connection_callback_(conn);
    close_callback_(conn);
}

void moony::tcp_connection::handle_error() {
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }

    LOG_FMT_ERROR("tcp_connection::handle_error name = %s, SO_ERROR = %d\n",
                     name_.c_str(), err);
}

