// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"
#include "EventLoop.h"

class EventLoopThread :noncopyable
{
public:
    EventLoopThread();//构造函数
    ~EventLoopThread();//析构
    EventLoop* startLoop();//启动事件循环，返回一个指针

private:
    void threadFunc();//线程函数
    EventLoop *loop_;//内部含有一个指向loop的指针，保证one thread one loop
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;//互斥锁
    Condition cond_;//条件变量
};