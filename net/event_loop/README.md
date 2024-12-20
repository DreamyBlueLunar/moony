## 关于 event_loop 的一些疑惑：
对于 event_loop::loop() 这个函数的逻辑，主要是集中在 do_pending_functions() 这一步调用上。
具体的问题有三个：
1. 为什么需要在 for 循环轮询过 active_channels_ 之后还要执行回调操作？
2. run_in_loop() 和 queue_in_loop() 是什么东西，为什么需要这两个成员函数，它会在哪里被调用？
3. 既然一个线程维护一个 event_loop 对象，那为什么在当前线程下向本线程中绑定的 loop->pending_functors_ 中插入的回调函数会在另一个线程中执行？
 
* A: 其实产生以上操作都是对 muduo 网络库的使用场景不够熟悉，而且也没有想清楚上层用户是怎么调用的。muduo保证on loop per thread的核心就是：每个线程（event_loop）都准备一个待执行队列，用于存储待执行函数，当其他线程需要调用某个不能跨线程调用的函数时（不能跨线程调用的函数比如有TCP_connection::send_in_loop()、timer_queue::add_timer_in_loop()、TCP_connection::shutdown_in_loop()、TCP_connection::connect_established()，因为他们都涉及到对fd的操作），只需要把该函数通过 event_loop::run_in_loop() 或者 event_loop::queue_in_loop() 加入到本线程的待执行队列中，然后由本线程从队列中取出该函数并执行。<br><br>
也就是说，某个线程调用了 TCP_connection::send()，如果这个线程是创建 event_loop 对象的线程，说明 TCP_connection::send() 这个调用没有发生跨线程的调用（每个 TCP_connection 都有个 loop_ 成员变量记录自己所属的 event_loop，event_loop 在构造的时候也会用 thread_id_ 记录下自己所属的线程，由于一个TCP_connection 只能对应一个线程，所以 loop_->is_in_loop_thread() 就能判断 loop_ 自己绑定的（或所属的）线程和当前调用 TCP_connection::send函数的线程是否是同一个线程），显然就应该在当前线程直接处理；否则就应该添加到调用 TCP_connection::send() 的线程的 event_loop::pending_functors_ 中，再唤醒相应线程，执行事件循环处理回调。而这里添加进 event_loop::pending_functors_ 的回调并不是由fd 直接通知线程的，所以 poller 没有办法监听到这些事件，自然在 for 循环之后还要调用一次 do_pending_functions()。