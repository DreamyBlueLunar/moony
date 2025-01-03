#pragma once

#include "../../base/copyable.h"

#include <string>
#include <vector>
#include <stddef.h>
#include <algorithm>

namespace moony {
class buffer : public copyable {
public:
    static const size_t k_cheap_prepend_ = 8;
    static const size_t k_initial_size_ = 1024;

    buffer(size_t k_initial_size = k_initial_size_):
        buffer_(k_initial_size),
        reader_idx_(k_cheap_prepend_),
        writer_idx_(k_cheap_prepend_) {}

    ~buffer() {}

    size_t readable_bytes() const { return writer_idx_ - reader_idx_;   }
    size_t writable_bytes() const {    return buffer_.size() - writer_idx_;    }
    size_t prependable_bytes() const {    return reader_idx_;    }

    // 返回缓冲区中可读数据的起始地址
    const char* peek() const {  return begin() + reader_idx_;   }

    void retrieve(size_t len) {
        if (len < readable_bytes()) { // 只读取了缓冲区中的部分数据
            reader_idx_ += len;
        } else {
            retrieve_all();
        }
    }

    void retrieve_all() {
        reader_idx_ = k_cheap_prepend_;
        writer_idx_ = k_cheap_prepend_;
    }

    // 把 on_message() 函数上报的 buffer 数据转成 std::string 类型数据返回
    std::string retrieve_all_as_string() {
        return retrieve_as_string(readable_bytes()); // 可读数据长度
    }

    std::string retrieve_as_string(size_t len) {
        std::string res(peek(), len);
        retrieve(len); // 上一步已经读走了缓冲区中的可读数据，这里就做复位操作
        return res;
    }

    void ensure_writeable_bytes(size_t len) {
        if (writable_bytes() < len) {
            make_space(len);
        }
    }

    // [data，data + len]区间上的数据添加到 buffer 当中
    void append(const char* data, size_t len) {
        ensure_writeable_bytes(len);
        std::copy(data, data + len, begin_write());
        writer_idx_ += len;
    }

    // 从 fd 上读取数据
    ssize_t read_fd(int fd, int* save_errno);
    // 向 fd 中写入数据
    ssize_t write_fd(int fd, int* save_errno);

private:
    char* begin() {
        return &*buffer_.begin();
    }

    const char* begin() const {
        return &*buffer_.begin();
    }

    char* begin_write() {
        return begin() + writer_idx_;
    }

    const char* begin_write() const {
        return begin() + writer_idx_;
    }

    void make_space(size_t len) {
        if (writable_bytes() + prependable_bytes() < 
                len + k_cheap_prepend_) {
            buffer_.resize(writer_idx_ + len);
        } else {
            size_t readable = readable_bytes();
            std::copy(begin() + reader_idx_,
                        begin() + writer_idx_,
                        begin() + k_cheap_prepend_);
            reader_idx_ = k_cheap_prepend_;
            writer_idx_ = reader_idx_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t reader_idx_;
    size_t writer_idx_;
};



}