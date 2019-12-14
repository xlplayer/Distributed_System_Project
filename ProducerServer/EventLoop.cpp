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
using std::bind;
using std::make_shared;

extern const int MAXFDS;
extern int socket_bind(int port);

EventLoop::EventLoop()
:_wakeupfd(eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC))
,_listenfd(socket_bind(0))
,_epoll(new Epoll())
,_mutex()
{
    if(listen(_listenfd, 100) < 0)
        perror("listen failed.");
    setSocketNonBlocking(_listenfd);
    _redis = redisConnect("127.0.0.1", 6379);
}

EventLoop::~EventLoop()
{
    close(_listenfd);
    close(_wakeupfd);
}

void EventLoop::handleConnect()
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    while( (clientfd = accept(_listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len)) > 0)
    {
        #ifdef DEBUG 
        printf("<new connection from %s:%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        #endif
        if(clientfd >= MAXFDS)
        {
            close(clientfd);
            continue;
        }
        setSocketNonBlocking(clientfd);
        shared_ptr<Channel> channel(new Channel(shared_from_this(), clientfd));
        channel->setEvents(EPOLLIN|EPOLLOUT);
        _epoll->add_channel(channel);

        //addPendingFunctions(bind(&Epoll::add_channel, _epoll, channel));
    }
}

void EventLoop::loop()
{
    _wakeupChannel = make_shared<Channel>(shared_from_this(),_wakeupfd);
    _wakeupChannel->setEvents(EPOLLIN);
    _wakeupChannel->setReadHandler(bind(&EventLoop::handlewakeup, this));
    _epoll->add_channel(_wakeupChannel);

    _acceptChannel = make_shared<Channel>(shared_from_this(),_listenfd);
    _acceptChannel->setEvents(EPOLLIN);
    _acceptChannel->setReadHandler(bind(&EventLoop::handleConnect, this));
    _epoll->add_channel(_acceptChannel);

    struct sockaddr_in listenaddr;
    socklen_t listenaddr_len = sizeof(listenaddr);
    getsockname(_listenfd, (struct sockaddr *)&listenaddr, &listenaddr_len);
    _listen_ip = inet_ntoa(listenaddr.sin_addr);
    _listen_port = ntohs(listenaddr.sin_port);
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
}