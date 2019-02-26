// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include "Timer.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <sys/epoll.h>


class EventLoop;//引进外部类
class HttpData;//数据

class Channel
{
private:
    typedef std::function<void()> CallBack;//C++11 function对象，用于回调函数
    EventLoop *loop_;
    int fd_;//描述符，socket
    __uint32_t events_;
    __uint32_t revents_;
    __uint32_t lastEvents_;//32位整型变量，记录事件

    // 方便找到上层持有该Channel的对象
    std::weak_ptr<HttpData> holder_;

private:
    int parse_URI();
    int parse_Headers();
    int analysisRequest();
    //回调函数对象声明
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

public:
    Channel(EventLoop *loop);//与eventloop绑定
    Channel(EventLoop *loop, int fd);
    ~Channel();
    int getFd();
    void setFd(int fd);

    void setHolder(std::shared_ptr<HttpData> holder)//设置对象持有者
    {   //赋值给weak_ptr
        holder_ = holder;
    }
    std::shared_ptr<HttpData> getHolder()
    {   //从weak_ptr获取所指向的对象所占有的shared_ptr
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;//返回
    }

    void setReadHandler(CallBack &&readHandler)//参数是右值引用，实现移动语义
    {   //读处理器函数
        readHandler_ = readHandler;
    }
    void setWriteHandler(CallBack &&writeHandler)//移动语义
    {   
        writeHandler_ = writeHandler;
    }
    void setErrorHandler(CallBack &&errorHandler)
    {
        errorHandler_ = errorHandler;
    }
    void setConnHandler(CallBack &&connHandler)
    {
        connHandler_ = connHandler;
    }

    void handleEvents()//每个channel都有对应的一个处理器函数
    {   
        events_ = 0;
        if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
        {
            events_ = 0;
            return;
        }
        if (revents_ & EPOLLERR)//错误事件
        {   
            if (errorHandler_) errorHandler_();//调用错误处理器
            events_ = 0;//事件清零
            return;
        }
        if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {   //出现以上事件
            handleRead();
        }
        if (revents_ & EPOLLOUT)
        {   //写事件触发
            handleWrite();
        }
        handleConn();
    }
    void handleRead();
    void handleWrite();
    void handleError(int fd, int err_num, std::string short_msg);
    void handleConn();

    void setRevents(__uint32_t ev)//设置返回的就绪事件
    {   //直接赋值
        revents_ = ev;
    }

    void setEvents(__uint32_t ev)
    {
        events_ = ev;
    }
    __uint32_t& getEvents()
    {
        return events_;
    }

    bool EqualAndUpdateLastEvents()
    {
        bool ret = (lastEvents_ == events_);//比较是否相等
        lastEvents_ = events_;//更新数值
        return ret;
    }

    __uint32_t getLastEvents()
    {   //直接返回变量
        return lastEvents_;
    }

};

typedef std::shared_ptr<Channel> SP_Channel;