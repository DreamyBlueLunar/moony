#pragma once

namespace lee {
/* 
 * 设计0：noncopyable
 * 禁用拷贝构造函数和赋值操作，隐藏掉默认构造函数和析构函数
 * 使得其派生类可以正常构造和析构，但是不能通过拷贝构造函数和赋值进行初始化
 * 同时也可以减少代码量
 */
class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
}