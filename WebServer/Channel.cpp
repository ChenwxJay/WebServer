// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Channel.h"
#include "Util.h"
#include "Epoll.h"
#include "EventLoop.h"
#include <unistd.h>
#include <queue>
#include <cstdlib>
#include <iostream>
using namespace std;

Channel::Channel(EventLoop *loop):
    loop_(loop),//绑定eventloop
    events_(0),
    lastEvents_(0)
{ }

Channel::Channel(EventLoop *loop, int fd)://构造函数
    loop_(loop),
    fd_(fd),//设置文件描述符
    events_(0),//绑定的事件，初始为空
    lastEvents_(0)
{ }

Channel::~Channel()
{
    //loop_->poller_->epoll_del(fd, events_);
    //close(fd_);
}

int Channel::getFd()//获取fd
{
    return fd_;
}
void Channel::setFd(int fd)//设置fd
{
    fd_ = fd;
}

void Channel::handleRead()//绑定在channel上面的读处理函数
{
    if (readHandler_)//判空
    {   //仿函数调用
        readHandler_();
    }
}

void Channel::handleWrite()
{
    if (writeHandler_)
    {
        writeHandler_();
    }
}

void Channel::handleConn()//连接处理函数
{   //连接回调函数
    if (connHandler_)
    {   
        connHandler_();
    }
}