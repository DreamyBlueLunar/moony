#include "../event_loop_thread.h"
#include "../../../base/current_thread/current_thread.h"
#include "../../event_loop/event_loop.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <unistd.h>

void print(moony::event_loop* p = NULL) {
    printf("print: pid = %d, tid = %d, loop = %p\n",
            getpid(), current_thread::tid(), p);
}

TEST(TEST_EVENT_LOOP_THREAD, TEST_EVENT_LOOP_THREAD_1) {
    moony::event_loop_thread th_1;
}

TEST(TEST_EVENT_LOOP_THREAD, TEST_EVENT_LOOP_THREAD_2) {
    moony::event_loop_thread th_2;
    moony::event_loop* lp_2 = th_2.start_loop();
    lp_2->run_in_loop(std::bind(print, lp_2));
    sleep(5);    

    moony::event_loop_thread th_3;
    moony::event_loop* lp_3 = th_3.start_loop();
    lp_3->run_in_loop(std::bind(print, lp_3));
    sleep(5);    
}