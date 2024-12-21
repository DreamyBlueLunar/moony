#include "../event_loop.h"
#include "../../../base/current_thread/current_thread.h"

#include <gtest/gtest.h>
#include <thread>

void callback() {
    printf("callback(): pid = %d, tid = %d\n", getpid(), current_thread::tid());
    moony::event_loop anotherLoop; // 应该在这里寄掉
}

void thread_entry() {
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), current_thread::tid());

    moony::event_loop loop;

    loop.run_in_loop(callback);
    loop.loop();
}

TEST(event_loop_test, test) {
    moony::event_loop loop;

    EXPECT_EQ(loop.in_loop_thread(), true);

    std::thread th(thread_entry);
    th.join();

    loop.loop();
}

