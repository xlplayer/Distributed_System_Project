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
#include "MessageQueue.h"
using std::queue;
using std::bind;
using namespace rapidjson;

extern MessageQueue msgQueue;

Channel::Channel(shared_ptr<EventLoop> eventLoop, int fd)
:_eventLoop(eventLoop)
,_epoll(eventLoop->getEpoll())
,_fd(fd)
,_state(CONNECTIING)
{
    struct timeval cur;
    gettimeofday(&cur,NULL);
    time_t time = cur.tv_sec;
    struct tm* p_time = localtime(&time);
    char s[26] = {0},t[40] = {0};
    strftime(s, 26, "%Y-%m-%d %H:%M:%S", p_time);
    snprintf(t,40,"%s:%ld",s,cur.tv_usec);
    _time = string(t,t+strlen(t));
    
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
    else
    {
        bool zero = false;
        int nread = readn(_fd, _readmsg, zero);
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(_readmsg[_readmsg.length()-nread+i]);
        printf("\n");
        printf("zero:%d\n",zero);
        Document d;
        d.Parse(_readmsg.c_str());
        if(zero) //got EOF
        {
            printf("client closed\n");
            _state = DISCONNTING;
        }
        if(d.IsObject())
        {
            string operate = d["operate"].GetString();
            if(operate == "reply")
            {
                int request_fd = d["fd"].GetInt();
                shared_ptr<Channel> channel = _epoll->get_channel(request_fd);
                printf("DDD\n");
                string time = d["time"].GetString();
                if(time == channel->getTime())
                {
                    channel->setWritemsg(_readmsg);
                    channel->setEvents(EPOLLOUT);
                    channel->update();
                    printf("AAA\n");
                }
            }
            else
            {         
                StringBuffer s;
                Writer<StringBuffer> writer(s);
                writer.StartObject();
                writer.Key("type");
                writer.String("push");
                writer.Key("time");
                writer.String(_time.c_str());
                writer.Key("ip");
                writer.String(_eventLoop->getListenip().c_str());
                writer.Key("port");
                writer.Int(_eventLoop->getListenport());
                writer.Key("fd");
                writer.Int(_fd);
                writer.Key("message");
                writer.String(_readmsg.c_str());
                writer.EndObject();       
                msgQueue.enqueue(Message(shared_from_this(),s.GetString()));
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

void Channel::addToEpoll()
{
    //shared_ptr<Epoll> epoll = _eventLoop->getEpoll();
    //epoll->add_channel(shared_from_this());
}