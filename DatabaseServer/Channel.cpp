#include "Channel.h"
#include "Socket.h"
#include "Epoll.h"
#include "Database.h"
#include "EventLoop.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include <sys/epoll.h>
#include <queue>
#include <unistd.h>
#include <sys/time.h>
#include <mysql/mysql.h>
using std::queue;
using std::bind;
using namespace rapidjson;

Channel::Channel(shared_ptr<EventLoop> eventLoop, int fd)
:_eventLoop(eventLoop)
,_epoll(eventLoop->getEpoll())
,_database(eventLoop->getDatabase())
,_msgQueue(eventLoop->getQueue())
,_fd(fd)
,_state(CONNECTIING)
,_curlyCount(0)
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
                if(operate == "query_train")
                {
                    string start,end;
                    start = d["start"].GetString();
                    end = d["end"].GetString();
                    vector<string> trains;
                    _database->getTrains(start, end, trains);

                    Value array(kArrayType);
                    for(size_t i = 0; i<trains.size();i++)
                        array.PushBack(StringRef(trains[i].c_str()), d.GetAllocator());
                    d.RemoveMember("result");
                    d.AddMember("result",array,d.GetAllocator());
                    StringBuffer s;
                    Writer<StringBuffer> writer(s);
                    d.Accept(writer);
                    _writemsg += s.GetString();

                }
                else if(operate == "query_leftnum")
                {
                    string train, date, start, end;
                    train = d["train"].GetString();
                    date = d["date"].GetString();
                    start = d["start"].GetString();
                    end = d["end"].GetString();
                    string result;
                    _database->getLeftNums(train, date, start, end, result);

                    d.RemoveMember("result");
                    d.AddMember("result",StringRef(result.c_str()),d.GetAllocator());
                    StringBuffer s;
                    Writer<StringBuffer> writer(s);
                    d.Accept(writer);
                    _writemsg += s.GetString();
                }
                else if(operate == "buy")
                {
                    string date,train_number,start,end,passenger_name,passenger_id;
                    date = d["date"].GetString();
                    train_number = d["train_number"].GetString();
                    start = d["start"].GetString();
                    end = d["end"].GetString();
                    passenger_name = d["passenger_name"].GetString();
                    passenger_id = d["passenger_id"].GetString();
                    string result;
                    _database->buyTicket(date, train_number, start, end, passenger_name, passenger_id, result);
                    #ifdef DEBUG 
                    printf("result: %s\n",result.c_str());
                    #endif
                    Document d_tmp;
                    d_tmp.Parse(result.c_str());
                    result = d_tmp["result"].GetString();
                    if(result == "failure")
                    {
                        d.AddMember("result", "failure", d.GetAllocator());
                    }
                    else if(result == "success")
                    {
                        d.AddMember("result", "success", d.GetAllocator());
                        d.AddMember("compartment_number", StringRef(d_tmp["compartment_number"].GetString()), d.GetAllocator());
                        d.AddMember("seat_number", StringRef(d_tmp["seat_number"].GetString()), d.GetAllocator());
                        d.AddMember("id", StringRef(d_tmp["id"].GetString()), d.GetAllocator());
                    }
                    StringBuffer s;
                    Writer<StringBuffer> writer(s);
                    d.Accept(writer);
                    _writemsg += s.GetString();
                }
                else if(operate == "pay")
                {
                    string date,train_number,id;
                    date = d["date"].GetString();
                    train_number = d["train_number"].GetString();
                    id = d["id"].GetString();
                    string result;
                    _database->payTicket(date, train_number, id, result);
                    #ifdef DEBUG 
                    printf("result: %s\n",result.c_str());
                    #endif
                    d.AddMember("result", StringRef(result.c_str()), d.GetAllocator());
                    StringBuffer s;
                    Writer<StringBuffer> writer(s);
                    d.Accept(writer);
                    _writemsg += s.GetString();
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