#pragma once

namespace lee {
/* 
 * 设计1：copyable，类似于noncopyable，但功能显然是相反的
 */
class copyable {
protected:
    copyable() = default;
    ~copyable() = default;
};
}