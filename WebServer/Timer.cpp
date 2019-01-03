// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Timer.h"
#include <sys/time.h> //与时间有关
#include <unistd.h> 
#include <queue> //队列


TimerNode::TimerNode(std::shared_ptr<HttpData> requestData, int timeout)
:   deleted_(false),//未删除
    SPHttpData(requestData)
{
    struct timeval now;//时间结构体
    gettimeofday(&now, NULL);//获取当前时间
    // 以毫秒计
	//设置定时器初始到时时间
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode()//析构函数
{
    if (SPHttpData)//内部的HttpData对象
        SPHttpData->handleClose();//处理关闭
}

TimerNode::TimerNode(TimerNode &tn)://拷贝构造
    SPHttpData(tn.SPHttpData)//直接调用构造函数
{ }


void TimerNode::update(int timeout)//更新定时器
{   //注意timeval结构体
    struct timeval now;
    gettimeofday(&now, NULL);//获取当前时间
	//更新定时时间
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::isValid()//是否有效
{
    struct timeval now;
    gettimeofday(&now, NULL);
	//通过now结构体获取对应的事件
    size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if (temp < expiredTime_)//小
        return true;
    else
    {   //定时器到期，已失效，删除
        this->setDeleted(); 
        return false;
    }
}

void TimerNode::clearReq()
{
    SPHttpData.reset();//shared_ptr释放管理的对象
    this->setDeleted();//删除
}


TimerManager::TimerManager()//构造函数
{ }

TimerManager::~TimerManager()//析构函数
{ }

void TimerManager::addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout)
{   //添加定时器，参数为一个shared_ptr, 以及定时时间
    SPTimerNode new_node(new TimerNode(SPHttpData, timeout));//创建对象，交由shared_ptr管理
    timerNodeQueue.push(new_node);//插入队列
    SPHttpData->linkTimer(new_node);//绑定定时器节点
}


/* 处理逻辑是这样的~
因为(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为deleted的时间节点，会延迟到它(1)超时 或 (2)它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
(1) 第一个好处是不需要遍历优先队列，省时。
(2) 第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
*/

void TimerManager::handleExpiredEvent()//处理定时
{
    //MutexLockGuard locker(lock);
    while (!timerNodeQueue.empty())//只要队列不为空
    {
        SPTimerNode ptimer_now = timerNodeQueue.top();//获取堆顶元素，使用优先队列
        if (ptimer_now->isDeleted())//判断是否可以删除
            timerNodeQueue.pop();
        else if (ptimer_now->isValid() == false)//判断是否有效
            timerNodeQueue.pop();//删除
        else
            break;
    }
}