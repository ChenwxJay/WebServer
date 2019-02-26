// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numThreads)
:   baseLoop_(baseLoop),
    started_(false),//停止
    numThreads_(numThreads),
    next_(0)//从第一个事件循环开始
{
    if (numThreads_ <= 0)
    {   //异步日志写入，发生错误
        LOG << "numThreads_ <= 0";
        abort();//退出程序
    }
}

void EventLoopThreadPool::start()//启动线程池函数
{   
    baseLoop_->assertInLoopThread();
    started_ = true;//设置标志位
    for (int i = 0; i < numThreads_; ++i)//循环启动多个线程
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());//创建一个EventLoop，并交由shared_ptr管理
        threads_.push_back(t);//添加到队列
        loops_.push_back(t->startLoop());//启动事件循环，并获取对应的loop指针，添加到队列
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()//获取事件循环
{   
    baseLoop_->assertInLoopThread();
    assert(started_);//断言，判断是否开始运行
    EventLoop *loop = baseLoop_;
    if (!loops_.empty())//判空
    {   
        loop = loops_[next_];//next指针
        next_ = (next_ + 1) % numThreads_;//循环到下一个，轮询
    }
    return loop;//返回下一个需要执行的event loop
}