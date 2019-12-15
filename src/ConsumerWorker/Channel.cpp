#include "Channel.h"
#include "base/Socket.h"
#include "Epoll.h"
#include "EventLoop.h"
#include "base/rapidjson/stringbuffer.h"
#include "base/rapidjson/document.h"
#include "base/rapidjson/writer.h"
#include <sys/epoll.h>
#include <queue>
#include <unistd.h>
#include <sys/time.h>
using std::queue;
using std::bind;
using namespace rapidjson;

const int MAXQUEUESIZE = 102400;

Channel::Channel(shared_ptr<EventLoop> eventLoop, int fd)
:_eventLoop(eventLoop)
,_epoll(eventLoop->getEpoll())
,_fd(fd)
,_state(CONNECTIING)
{
    #ifdef DEBUG 
    printf("channel newed :%d\n",_fd);
    #endif
}

Channel::~Channel()
{
    close(_fd);
    #ifdef DEBUG 
    printf("channel released :%d\n",_fd);
    #endif
}

void Channel::handleRead()
{
    if(_readHandler) _readHandler();  
}   

void Channel::handleWrite()
{
    if(_writerHandler) _writerHandler();
    else
    {
        #ifdef DEBUG
        if(_writemsg.size()>0) printf("response: %s\n", _writemsg.c_str());
        #endif
        writen(_fd, _writemsg);
    }
    
}

void Channel::handleEvents()
{
    if(_revents & EPOLLIN)
        handleRead();
    if(_revents & EPOLLOUT)
        handleWrite();

    if(_state == DISCONNTING)
    {
        _state = DISCONNTED;
        _eventLoop->addPendingFunctions(bind(&Epoll::del_channel, _epoll, shared_from_this()));
    }
}

void Channel::update()
{
    _epoll->mod_channel(shared_from_this());
}