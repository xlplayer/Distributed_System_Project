#include "Epoll.h"
#include "Channel.h"
#include <unistd.h>

Epoll::Epoll()
:_epollfd(epoll_create1(EPOLL_CLOEXEC))
,_events(MAXEVENTS)
{}

Epoll::~Epoll()
{}

void Epoll::add_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    #ifdef DEBUG 
    printf("add fd:%d\n",fd);
    #endif
    if(epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev)<0)
    {
        perror("epoll add errpr");
    }
}

void Epoll::mod_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev)<0)
    {
        perror("epoll mod error");
    }    
}

void Epoll::del_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &ev)<0)
    {
        perror("epoll del error");
    } 
    #ifdef DEBUG 
    printf("del %d\n",fd);    
    #endif
}

void Epoll::add_channel(shared_ptr<Channel> channel)
{
    int fd = channel->getFd();
    uint32_t events = channel->getEvents();
    add_event(fd, events);
    _fd2channel[fd] = channel;
    
}

void Epoll::mod_channel(shared_ptr<Channel> channel)
{
    int fd = channel->getFd();
    uint32_t events = channel->getEvents();
    mod_event(fd, events);
    _fd2channel[fd] = channel;
    
}

void Epoll::del_channel(shared_ptr<Channel> channel)
{
    int fd = channel->getFd();
    uint32_t events = channel->getEvents();
    del_event(fd, events);
    _fd2channel[fd].reset();
    //close(fd); channel colse when release
}

void Epoll::handle_activate_channels()
{
    int num = epoll_wait(_epollfd, &*_events.begin(), _events.size(), -1);
    for(int i=0; i<num; i++)
    {
        int fd = _events[i].data.fd;
        shared_ptr<Channel> channel = _fd2channel[fd];
        channel->setRevents(_events[i].events);
        channel->handleEvents();
    }
}