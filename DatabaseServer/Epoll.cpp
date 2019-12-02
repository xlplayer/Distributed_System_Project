#include "Epoll.h"


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
    epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev);    
}

void Epoll::mod_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev);        
}

void Epoll::del_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &ev);    
}

void Epoll::wait_event()
{
    _eventNum = epoll_wait(_epollfd, &*_events.begin(), _events.size(), -1);
}
