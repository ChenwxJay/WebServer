// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include "HttpData.h"
#include "base/noncopyable.h"
#include "base/MutexLock.h"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>

class HttpData;//声明外部类

class TimerNode
{
public:
    TimerNode(std::shared_ptr<HttpData> requestData, int timeout);//某个http请求和超时参数
    ~TimerNode();
    TimerNode(TimerNode &tn);//拷贝构造函数
    void update(int timeout);//更新
    bool isValid();//是否有效
    void clearReq();
    void setDeleted() { deleted_ = true; }//设置标志位
    bool isDeleted() const { return deleted_; }
    size_t getExpTime() const { return expiredTime_; }//获取时间值 
private:
    bool deleted_;//是否可以删除
    size_t expiredTime_;//unsigned int
    std::shared_ptr<HttpData> SPHttpData;//绑定HttpData
};

struct TimerCmp//仿函数
{
    bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
    {   //比较定时时间，用于优先队列
        return a->getExpTime() > b->getExpTime();
    }
};

class TimerManager
{
public:
    TimerManager();
    ~TimerManager();
	//给每个http请求添加一定超时时间的定时器
    void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
    void handleExpiredEvent();//处理到期事件

private:
    typedef std::shared_ptr<TimerNode> SPTimerNode;//一个智能指针
	//注意优先队列容器是一个模板类，第三个模板参数接收一个比较器
    std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp> timerNodeQueue;//优先队列，底层使用deque
    //MutexLock lock;
};