#include "../base_thread.h"
#include "../../current_thread/current_thread.h"

#include <gtest/gtest.h>
#include <iostream>

void func(int a, int b) {
    int res = a + b;
    std::cout << "func(" << a << ", "
             << b << "): " << res << std::endl;
    std::cout << "now quit thread " << current_thread::tid() << std::endl;
}

TEST(test_base_thread, test_base_2_threads) {
    moony::base_thread th(std::bind(&func, 1, 2), "test");
    th.start();
    std::cout << "thread th's id: " << th.tid() << std::endl;
    th.join();
    std::cout << "now quit main thread " << current_thread::tid() << std::endl;
}

TEST(test_base_thread, test_base_20_threads) {
    for (int i = 0; i < 20; i ++) {
        moony::base_thread th(std::bind(&func, 1, 2), "test");
        std::cout << "thread th's name: " << th.name() << std::endl;
        th.start();
        std::cout << "thread th's id: " << th.tid() << std::endl;
        th.join();
    }
    std::cout << moony::base_thread::num_created() << " threads created" << std::endl;
    std::cout << "now quit main thread " << current_thread::tid() << std::endl;
}