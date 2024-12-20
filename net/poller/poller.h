#pragma once

#include "../../base/noncopyable.h"
#include "../../base/time_stamp/time_stamp.h"

#include <vector>
#include <map>

namespace lee {
class event_loop;
class channel;

/**
 * event_loop、channel、poller之间的关系？
 * A: 一个线程对应一个 event_loop，一个 event_loop 由一个 poller 实现，
 * 而这个线程可以是主线程，可以是工作线程，显然一个线程处理的事件不止一个，
 * 所以可以对应多个 channel。这里的channel显然可以看作是 reactor 感兴趣的事件的封装。
 * 
 * poller 封装了一个 I/O复用接口，还维护了一个存放 channel 的 map，对应 Reactor 模式中的 Demultiplexer
 */
class poller : noncopyable {
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