#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace current_thread {
extern __thread int t_cached_tid;

void cache_tid();

inline int tid() {
    if (__builtin_expect(t_cached_tid == 0, 0)) {
        cache_tid();
    }
    return t_cached_tid;
}
}