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
#include <hiredis/hiredis.h>
#include "MessageQueue.h"
using std::queue;
using std::bind;
using namespace rapidjson;

extern MessageQueue msgQueue;

Channel::Channel(shared_ptr<EventLoop> eventLoop, int fd)
:_eventLoop(eventLoop)
,_epoll(eventLoop->getEpoll())
,_redis(eventLoop->getRedis())
,_fd(fd)
,_state(CONNECTIING)
,_curlyCount(0)
{
    struct timeval cur;
    gettimeofday(&cur,NULL);
    time_t time = cur.tv_sec;
    struct tm* p_time = localtime(&time);
    char s[26] = {0},t[40] = {0};
    strftime(s, 26, "%Y-%m-%d %H:%M:%S", p_time);
    snprintf(t,40,"%s:%ld",s,cur.tv_usec);
    _time = string(t,t+strlen(t));

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
        #ifdef DEBUG 
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(_readmsg[_readmsg.length()-nread+i]);
        printf("\n");
        printf("zero:%d\n",zero);
        #endif
        if(zero || nread == -1) //got EOF or close force
        {
            #ifdef DEBUG 
            printf("client closed\n");
            #endif
            _state = DISCONNTING;
        }

        Document d;
        while(!_readmsg.empty())
        {
            int len;
            for(len=0;len<_readmsg.length();len++) //count { and }
            {
                if(_readmsg[len]=='{') _curlyCount++;
                else if(_readmsg[len]=='}') _curlyCount--;
                if(_curlyCount == 0) break;
            } 
            if(_curlyCount == 0)
            {
                d.Parse(_readmsg.substr(0,len+1).c_str());
                string operate = d["operate"].GetString();
                if(operate == "reply")
                {
                    int request_fd = d["fd"].GetInt();
                    shared_ptr<Channel> channel = _epoll->get_channel(request_fd);
                    string time = d["time"].GetString();
                    if(channel && time == channel->getTime())
                    {
                        string &msg = channel->getWritemsg();
                        msg += _readmsg.substr(0, len+1);
                    }
                    if(d.HasMember("train_num")) //cache
                    {
                        redisCommand(_redis,"sadd %s_%s_%s %s",d["date"].GetString(),d["start"].GetString(),d["end"].GetString(),_readmsg.substr(0, len+1).c_str());
                    }
                }
                else if(operate == "query")
                {
                    redisReply* reply = (redisReply*)redisCommand(_redis, "smembers %s_%s_%s",d["date"].GetString(),d["start"].GetString(),d["end"].GetString());
                    if(reply == NULL || reply->elements == 0)
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
                    else if(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
                    {
                        d["operate"].SetString("reply");
                        d.AddMember("train_num", 0, d.GetAllocator());
                        d.AddMember("result","", d.GetAllocator());
                        for(int i=0;i<reply->elements;i++)
                        {
                            Document d_tmp;
                            d_tmp.Parse(reply->element[i]->str);  
                            d["train_num"].SetUint(d_tmp["train_num"].GetUint());
                            d["result"].SetString(StringRef(d_tmp["result"].GetString()));
                            StringBuffer s;
                            Writer<StringBuffer> writer(s);
                            d.Accept(writer);
                            _writemsg += s.GetString();
                        }
                    }
                }
                else if(operate == "buy" || operate == "pay")
                {    
                    if(operate == "buy")//remove cache
                    {
                        redisCommand(_redis,"del %s_%s_%s",d["date"].GetString(),d["start"].GetString(),d["end"].GetString());
                    }     
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

                _readmsg = _readmsg.substr(len+1);
                continue;
            } 
            break;    
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
            #ifdef DEBUG 
            printf("writemsg: %s\n",_writemsg.c_str());
            #endif
            writen(_fd, _writemsg); 
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