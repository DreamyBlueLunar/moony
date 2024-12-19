#include "epoll_poller.h"
#include "../../base/logger/logger.h"
#include "../../base/time_stamp/time_stamp.h"
#include "../channel/channel.h"

#include <errno.h>
#include <unistd.h>
#include <strings.h>

namespace {
    const int k_new = -1; // channel 未添加到 poller 中
    const int k_added = 1;  // channel 已添加到 poller 中
    const int k_deled = 2;  // channel 从 poller 中删除
}

lee::epoll_poller::epoll_poller(event_loop* loop): 
         poller(loop),
         epoll_fd_(::epoll_create(1)),
         events_(k_init_event_list_size) {
    if (epoll_fd_ < 0) {
        LOG_FMT_FATAL("epoll_create error: %d\n", errno);
    }
}

lee::epoll_poller::~epoll_poller() {
    ::close(epoll_fd_);
}

// 重写 poller 基类的纯虚函数
lee::time_stamp lee::epoll_poller::poll_wait(int timeout_ms, 
        channel_list* active_channels) {
    // LOG_FMT_DEBUG("");
    LOG_FMT_INFO("func = %s -> fd count: %ld\n", __FUNCTION__, channels_.size());

    int num_events = ::epoll_wait(epoll_fd_, 
                                  &*events_.begin(),
                                  static_cast<int>(events_.size()),
                                  timeout_ms);
    int save_errno = errno; // 记录当前的错误类型
    time_stamp now(time_stamp::now());
    if (num_events > 0) {
        LOG_FMT_INFO("%d events happened\n", num_events);
        fill_active_channels(num_events, active_channels);
        if (num_events == events_.size()) {
            events_.resize(2 * num_events);
        }
    } else if (0 == num_events) {
        LOG_FMT_DEBUG("no events happened\n");      
    } else {
        if (save_errno != EINTR) {
            errno = save_errno;
            LOG_FMT_ERROR("epoll_poller::poll_wait(): %d\n", errno);
        }
    }

    return now;
}

// 通过 update() 函数更改 channel
void lee::epoll_poller::update_channel(channel* chann) {
    const int index = chann->index();
    int fd = chann->fd();

    LOG_FMT_INFO("func = %s -> fd = %d, events = %d, index = %d\n", 
            __FUNCTION__, chann->fd(), chann->events(), chann->index());

    if (index == k_new || index == k_deled) {
        if (index == k_new) {
            channels_[fd] = chann;
        }
        chann->set_index(k_added);
        update(EPOLL_CTL_ADD, chann);
    } else {
        if (chann->is_none_event()) {    
            update(EPOLL_CTL_DEL, chann);
            chann->set_index(k_deled);
        } else {
            update(EPOLL_CTL_MOD, chann);
        }
    }
}

// 通过 update() 函数删除 channel
void lee::epoll_poller::remove_channel(channel* chann) {
    int fd = chann->fd();
    int index = chann->index();

    LOG_FMT_INFO("func = %s -> fd = %d, events = %d, index = %d\n", 
            __FUNCTION__, chann->fd(), chann->events(), chann->index());


    size_t n = channels_.erase(fd);
    if (index == k_added) {
        update(EPOLL_CTL_DEL, chann);
    }
    chann->set_index(k_new);
}

// 填写活跃的 channels
void lee::epoll_poller::fill_active_channels(int num_events, 
        channel_list* active_channels) const {
    for (int i = 0; i < num_events; i ++) {
        channel* chann = static_cast<channel*>(events_[i].data.ptr);
        chann->set_revents(events_[i].events);
        active_channels->push_back(chann);
    }
}

// 更新 channel，调用 epoll_ctl 做具体的操作
void lee::epoll_poller::update(int op, channel* chann) {
    epoll_event ev;
    bzero(&ev, sizeof(epoll_event));
    ev.data.ptr = chann;
    ev.data.fd = chann->fd();
    ev.events |= chann->events();
    int fd = chann->fd();

    if (::epoll_ctl(epoll_fd_, op, fd, &ev) < 0) {
        if (op == EPOLL_CTL_DEL) {
            LOG_FMT_ERROR("epoll_ctl op = DEL, fd = %d\n", fd);
        } else { // 如果add或者mod出了问题，显然系统没有办法继续使用
            LOG_FMT_FATAL("epoll_ctl op = ADD/MOD, fd = %d\n", fd);
        }
    }
}