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

Channel::Channel(shared_ptr<EventLoop> eventLoop, int fd)
:_eventLoop(eventLoop)
,_epoll(eventLoop->getEpoll())
,_msgQueue(eventLoop->getQueue())
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
    else
    {
        bool zero = false;
        int nread = readn(_fd, _readmsg, zero);

        Document d;
        d.Parse(_readmsg.c_str());
        if(zero || nread == -1) //got EOF or close force
        {
            #ifdef DEBUG 
            printf("client closed\n");
            #endif
            _state = DISCONNTING;
        }
        if(d.IsObject())
        {
            string type = d["type"].GetString();
            if(type == "push")
            {
                if(_msgQueue.size() >= 1024)
                {
                    setWritemsg("{\"result\":\"push failure\"}");
                    setEvents(EPOLLOUT);
                    update();
                }
                else
                {
                    setWritemsg("{\"result\":\"push success\"}");
                    setEvents(EPOLLIN|EPOLLOUT);
                    update();
                    _msgQueue.push(_readmsg);
                }
            }
            else if(type == "pop")
            {         
                if(!_msgQueue.empty())
                {
                    string str = _msgQueue.front();
                    _msgQueue.pop();
                    setWritemsg(str);
                    setEvents(EPOLLOUT);
                    update();
                }
                else
                {
                    setWritemsg("{\"result\":\"pop failure\"}");
                    setEvents(EPOLLOUT);
                    update();
                }
            }
            _readmsg.clear();       
        }    
    }           
}   

void Channel::handleWrite()
{
    if(_writerHandler) _writerHandler();
    else
    {
        if(!_writemsg.empty())
        {
            int nwrite = writen(_fd, _writemsg);
            if(_writemsg.empty())
            {
                setEvents(EPOLLIN);
                update();
            }
                
        }
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