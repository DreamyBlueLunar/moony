# moony

## muduo 网络库的设计与其在 muduo 中对应的组件
理论上，每一个线程（thread）对应一个事件循环（event loop），每一个事件循环对应一个I/O复用接口（select、poll、epoll），每一个I/O复用接口监听多个文件描述符（fd），而一个事件（event）对应一个 fd，也就是说，一个I/O复用接口监听多个该线程感兴趣的事件。<br>

对应到 muduo 中的组件，我们可以发现：<br>

### channel (event)
事实上 channel 中封装了 fd 和其感兴趣的事件的类型，如EPOLLIN、EPOLLOUT，封装了各类事件的回调，还封装了一个 poller 用于监听 fd 上发生的事件，有一个 loop 成员用于指示拥有该 channel 的事件循环。通过这个描述不难发现其实它对应的是 Reactor 模式中的事件（event），它通过 enable_reading() 等接口向 I/O 复用接口中注册事件，通过 handle_event() 接口调用事件的回调函数处理事件。channel 在析构之前要调用 event_loop::remove_channel()，避免悬空指针。<br>

### poller 和 epoll_poller (demultiplexer)
poller 是对 I/O 复用接口的一层抽象封装，定义成了一个抽象基类，后面可以实现 epoll_poller、poll_poller 等 poller。当用户通过 channel::enable_reading() 向 poller 中注册事件的时候，会通过 event_loop::update_channel() 调用 channel::update()。poller 中可以维护一个 std::map<channel*> 类型的字典，以存放 poller 感兴趣的 channel。epoll_poller 是 poller 的派生类。epoll_poller 类是基于 epoll 实现的一个 poller，其核心是 epoll_poller::poll_wait()，里面调用了 epoll_wait()。而 poller::update() 会再调用到 epoll_poller::update_channel()，最后调用到 epoll_poller::update() 执行最终的 ::epoll_ctl() 调用，实现事件的注册和注销。<br>

### event_loop (Reactor)
event_loop 中封装了: 需要处理的 channel、poller、wakeupfd 和 wakeup_channel。event_loop 充当 poller 和 channel 通信的桥梁，也就是Reactor模型中的 Reactor 模块。channel 和 poller 不能直接进行通信。如果想要把 channel 注册到 poller 上，需要找到拥有这个 channel 对象的 loop，通过 loop 再找到 poller，最后完成 ::epoll_ctl的调用；如果某个 channel 感兴趣的事件发生了，poller 也需要通过 loop 找到对应的 channel，再执行 channel 中注册的回调。wakeupfd 是通过系统调用 ::eventfd() 创建的，封装成 wakeup_channel。只要向 wakeupfd 中写入流，就可以唤醒 event_loop。<br>

### base_thread 和 event_loop_thread
base_thread 是对 std::thread 的一层封装，将线程的注册和创建分离开来，在调用了 base_thread::start() 之后才会开始跑新的线程。这样做的好处是可以自己掌控线程启动的时机；event_loop_thread 里面封装了一个 loop 和一个 base_thread，在调用到 event_loop_thread::start_loop() 的时候就会调用 base_thread::start() 启动新线程。<br>

### event_loop_thread_pool
线程池中封装了 get_next_loop()，通过轮询算法得到下一个可以使用的subloop（子线程）。这里也可以体现出 muduo 网络库 one loop per thread 的设计。<br>

### socket
封装了底层的各种 socket 相关的操作。<br>

### acceptor
主要封装了 listenfd 相关的操作。acceptor::listen() 将端口置为 LISTEN 状态，同时将 accept_channel_ 注册到 poller 中；acceptor::handle_read() 可以处理新到来的连接，作为 accept_channel 的 read_callback。也就是说，当有新连接事件的时候，会调用到acceptor::handle_read() 这里来，最终调用到 tcp_server::new_connection()。<br>

### buffer
应用写数据 -> 缓冲区 -> tcp 发送缓冲区 -> send
prependable(8)   readeridx   writeridx

### tcp_connection
一个连接成功的客户端对应一个 tcp_connection

### tcp_server
将所有的组件全部串起来