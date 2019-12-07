#include "Channel.h"
#include "Socket.h"
#include "Epoll.h"
#include "EventLoop.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
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
    printf("channel newed :%d\n",_fd);
}

Channel::~Channel()
{
    close(_fd);
    printf("channel released :%d\n",_fd);
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
        if(_writemsg.size()>0) printf("response: %s\n", _writemsg.c_str());
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