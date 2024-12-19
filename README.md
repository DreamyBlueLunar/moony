# moony

## Phase 1: Reactor 模式的理解与其在 muduo 中对应的组件
首先 Reactor 模式中，每一个线程（thread）对应一个事件循环（event loop），每一个事件循环对应一个I/O复用接口（select、poll、epoll），每一个I/O复用接口监听多个文件描述符（fd），而一个事件（event）对应一个 fd，也就是说，一个I/O复用接口监听多个该线程感兴趣的事件。<br>

对应到 muduo 中的组件，我们可以发现：<br>

### channel 类
事实上 channel 类中封装了 fd 和其感兴趣的事件的类型，如EPOLLIN、EPOLLOUT，封装了各类事件的回调，还封装了一个 poller 用于监听 fd 上发生的事件。通过这个描述不难发现其实它对应的是 Reactor 模式中的事件（event），它通过 enable_reading() 等接口向 I/O 复用接口中注册事件，通过 handle_event() 接口调用事件的回调函数处理事件。channel 在析构之前要调用 event_loop::remove_channel()，避免悬空指针。

### poller 类
poller 类是对 I/O 复用接口的一层抽象封装，定义成了一个抽象基类，后面可以实现 epoll_poller、poll_poller 等 poller。当用户通过 channel::enable_reading() 向 poller 中注册事件的时候，会通过event_loop::update_channel() 调用 channel::update()。显然 poller 类中应该维护一个 std::map<channel*> 类型的字典，以存放 poller 感兴趣的 channel。

### epoll_poller 类
poller 的派生类。epoll_poller 类是基于 epoll 实现的一个 poller，其核心是 epoll_poller::poll_wait()，里面调用了 epoll_wait()。而 poller::update() 会再调用到 epoll_poller::update_channel()，最后调用到 epoll_poller::update() 执行最终的 ::epoll_ctl() 调用，实现事件的注册和注销。

### event_loop 类