#include "current_thread.h"

namespace current_thread {
__thread int t_cached_tid = 0;

void cache_tid() {
    if (0 == t_cached_tid) {
        t_cached_tid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}
}