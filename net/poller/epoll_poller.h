#pragma once

#include "poller.h"
#include "../../base/time_stamp/time_stamp.h"

#include <sys/epoll.h>
#include <vector>

namespace lee {
class event_loop;
class channel;

class epoll_poller : public poller {
public:
    epoll_poller(event_loop* loop);
    ~epoll_poller() override;

    // 重写 poller 基类的纯虚函数
    time_stamp poll_wait(int timeout_ms, 
                            channel_list* active_channels) override;
    void update_channel(channel* chann) override;
    void remove_channel(channel* chann) override;

private:
    static const int k_init_event_list_size = 16;
    
    // 填写活跃的 channels
    void fill_active_channels(int num_events, 
                                channel_list* active_channels) const;
    // 更新 channel
    void update(int op, channel* chann);

    using event_list = std::vector<epoll_event>;

    int epoll_fd_;
    event_list events_;
};

}