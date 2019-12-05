#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "Socket.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>
#include <iostream>
using namespace std;
using std::bind;

extern const int MAXFDS;
extern int socket_bind(int port);

EventLoop::EventLoop()
:_wakeupfd(eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC))
,_epoll(new Epoll())
,_mutex()
{}

EventLoop::~EventLoop()
{
    close(_wakeupfd);
}

void EventLoop::loop()
{
    _wakeupChannel = make_shared<Channel>(shared_from_this(),_wakeupfd);
    _wakeupChannel->setEvents(EPOLLIN);
    _wakeupChannel->setReadHandler(bind(&EventLoop::handlewakeup, this));
    _epoll->add_channel(_wakeupChannel);

    while(1)
    {
        _epoll->handle_activate_channels();
        doPendingFunctions();
    }
}

void EventLoop::addPendingFunctions(function<void()> &&cb)
{
    MutexLockGuard lcok(_mutex);
    _pendingFunctions.push_back(cb);
}

void EventLoop::doPendingFunctions()
{
    vector<function<void()> >functions;
    {
        MutexLockGuard lock(_mutex);
        functions.swap(_pendingFunctions);
    }
    for(size_t i =0; i<functions.size();i++)
        functions[i]();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(_wakeupfd, &one, sizeof(one));
}

void EventLoop::handlewakeup()
{
    string msg;
    bool zero;
    int n = readn(_wakeupfd, msg, zero);
    printf("wakeupmsg:%s\n", msg.c_str());
}