#include "buffer.h"
#include <sys/uio.h>

/**
 * 从 fd 上读取数据，poller 工作在 LT 模式
 * 但是这里并不知道数据大小，但是 buffer 显然又有一个大小限制
 */
ssize_t moony::buffer::read_fd(int fd, int* save_errno) {
    char extrabuf[65536] = {0}; // 栈上内存，函数结束以后自动释放。
                                // 原本 buffer 中存不下的数据都被存到这段内存上，
                                // 然后再对 buffer 扩容，复制到 buffer 中
    iovec vec[2];

    const size_t writable = writable_bytes();
    vec[0].iov_base = begin() + writer_idx_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    int ret = ::readv(fd, vec, iovcnt);
    if (ret < 0) {
        *save_errno = errno;
    } else if (ret <= writable) { // 如果读取的数据少于 buffer 中可写空间，直接更新 writer_idx_
        writer_idx_ += ret;
    } else { // 否则就要 append 一段数据进去
        writer_idx_ += buffer_.size();
        append(extrabuf, ret - writable);
    }

    return ret;
}