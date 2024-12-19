#pragma once

#include "../../base/noncopyable.h"
#include "../../base/time_stamp/time_stamp.h"

#include <vector>
#include <map>

namespace lee {
class event_loop;
class channel;

class poller : lee::noncopyable {
public:
    using channel_list = std::vector<channel*>;

    poller(event_loop* loop): owner_loop_(loop) {}
    virtual ~poller() = default;

    // 封装 epoll_wait、poll 等 I/O 复用接口，线程中必须调用这个接口实现事件循环
    virtual time_stamp poll_wait(int timeout_ms, 
                                    channel_list* active_channels) = 0;
    // 更新感兴趣的事件，线程中必须调用这个接口
    virtual void update_channel(channel* chann) = 0;
    // 删除事件，线程中必须调用这个接口
    virtual void remove_channel(channel* chann) = 0;
    // 检查 channels_ 中有没有这个 channel
    virtual bool has_channel(channel* chann) const;

    static poller* new_default_poller(event_loop* loop);

protected:
    using channel_map = std::map<int, channel*>;
    channel_map channels_;

private:
    event_loop* owner_loop_;
};
}