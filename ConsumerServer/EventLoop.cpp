#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "Socket.h"
#include <unistd.h>
#include <string.h>
#include <sys/eventfd.h>
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/document.h"
using namespace rapidjson;
using std::bind;
using std::make_shared;

extern const int MAXFDS;
extern vector<sockaddr_in> queueServerAddr,databaseServerAddr;
extern int socket_bind(int port);

EventLoop::EventLoop()
:_curlyCount(0)
,_wakeupfd(eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC))
,_epoll(new Epoll())
,_mutex()
{
    int index = rand()%queueServerAddr.size();
    _queuefd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(_queuefd, (struct sockaddr*)&queueServerAddr[index], sizeof(queueServerAddr[index])) == -1)
    {
        perror("consumer connect queue failed.");
    }
    setSocketNonBlocking(_queuefd);

    index = rand()%databaseServerAddr.size();
    _databasefd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(_databasefd, (struct sockaddr*)&databaseServerAddr[index], sizeof(databaseServerAddr[index])) == -1)
    {
        perror("consumer connect database failed.");
    } 
    setSocketNonBlocking(_databasefd);
}

EventLoop::~EventLoop()
{
    close(_wakeupfd);
    close(_queuefd);
    close(_databasefd);
}

void EventLoop::loop()
{
    _wakeupChannel = make_shared<Channel>(shared_from_this(),_wakeupfd);
    _wakeupChannel->setEvents(EPOLLIN);
    _wakeupChannel->setReadHandler(bind(&EventLoop::handlewakeup, this));
    _epoll->add_channel(_wakeupChannel);

    _queueChannel = make_shared<Channel>(shared_from_this(),_queuefd);
    _queueChannel->setEvents(EPOLLOUT);
    _queueChannel->setReadHandler(bind(&EventLoop::handleQueueRead, this));
    _queueChannel->setWriterHandler(bind(&EventLoop::handleQueueWrite, this));
    _epoll->add_channel(_queueChannel);

    _databaseChannel = make_shared<Channel>(shared_from_this(),_databasefd);
    _databaseChannel->setEvents(EPOLLIN | EPOLLOUT);
    _databaseChannel->setReadHandler(bind(&EventLoop::handleDatabaseRead, this));
    _databaseChannel->setWriterHandler(bind(&EventLoop::handleDatabaseWrite, this));
    _epoll->add_channel(_databaseChannel);

    while(1)
    {
        _epoll->handle_activate_channels();
        doPendingFunctions();
    }
}

void EventLoop::addPendingFunctions(function<void()> &&cb)
{
    MutexLockGuard lock(_mutex);
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

void EventLoop::handleQueueRead()
{
    string &msg = _queueChannel->getReadmsg();
    bool zero = false;
    int nread = readn(_queuefd, msg, zero);
    if(zero || nread == -1) //got EOF or close force
    {
        #ifdef DEBUG 
        printf("client closed\n");
        #endif
        _queueChannel->setState(DISCONNTING);
    }

    Document d;  
    d.Parse(msg.c_str());
    if(d.IsObject())
    {
        if(msg == "{\"result\":\"pop failure\"}")
        {
            _queueChannel->setEvents(EPOLLOUT);
            _queueChannel->update();
            msg.clear();
            return;
        }

        #ifdef DEBUG 
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(msg[msg.length()-nread+i]);
        printf("\n");
        printf("zero:%d\n",zero);
        #endif
        
        string ip = d["ip"].GetString();
        int port = d["port"].GetInt();
        if(_addr2channel.find(make_pair(ip,port)) == _addr2channel.end())
        {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in servaddr;
            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
            if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) <0 )
                perror("connect error");  
            setSocketNonBlocking(sockfd); 
            shared_ptr<Channel> channel(new Channel(shared_from_this(), sockfd));
            channel->setEvents(EPOLLOUT);
            _epoll->add_channel(channel);
            _addr2channel[make_pair(ip,port)] = channel;   
        }
        _msgQueue.push(msg);
        _queueChannel->setEvents(EPOLLOUT);
        _queueChannel->update();

        msg.clear();
    }   
}

void EventLoop::handleQueueWrite()
{
    string &msg = _queueChannel->getWritemsg();
    if(!msg.empty())
    {
        int nwrite = writen(_queuefd, msg);
        if(msg.empty())
        {
            _queueChannel->setEvents(EPOLLIN);
            _queueChannel->update();
        }  
    }
    else
    {
        if(_msgQueue.size() < 100) 
        {
            msg = "{\"type\":\"pop\"}";
            int nwrite = writen(_queuefd, msg);
            if(msg.empty())
            {
                _queueChannel->setEvents(EPOLLIN);
                _queueChannel->update();
            }  
        }
    }
}

void EventLoop::handleDatabaseRead()
{
    string &msg = _databaseChannel->getReadmsg();
    bool zero = false;
    int nread = readn(_databasefd, msg, zero);
    if(nread > 0)
    {
        #ifdef DEBUG 
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(msg[msg.length()-nread+i]);
        printf("\n");
        printf("zero:%d\n",zero);
        #endif
    }

    if(zero || nread == -1) //got EOF or close force
    {
        #ifdef DEBUG 
        printf("client closed\n");
        #endif
        _databaseChannel->setState(DISCONNTING);
    }

    Document d;
    while(!msg.empty())
    {
        int len;
        for(len=0;len<msg.length();len++) //count { and }
        {
            if(msg[len]=='{') _curlyCount++;
            else if(msg[len]=='}') _curlyCount--;
            if(_curlyCount == 0) break;
        } 
        if(_curlyCount == 0)
        {
            d.Parse(msg.substr(0,len+1).c_str());
            Value &val = d["result"];
            string operate = d["operate"].GetString();
            if(operate == "query_train")
            {
                if(val.Size() != 0)
                {
                    d["operate"].SetString("query_leftnum");
                    d.AddMember("train_num", val.Size(), d.GetAllocator());
                    d.AddMember("train","",d.GetAllocator());
                    for(size_t i=0; i<val.Size();i++)
                    {
                        Value &v = val[i];
                        d["train"].SetString(StringRef(v.GetString()));
                        string &writemsg = _databaseChannel->getWritemsg();
                        StringBuffer s;
                        Writer<StringBuffer> writer(s);
                        d.Accept(writer);
                        writemsg += s.GetString();
                        //trains.push_back(v.GetString());
                    }
                }
                else //no trains
                {
                    string ip = d["ip"].GetString();
                    int port = d["port"].GetInt();
                    shared_ptr<Channel> channel = _addr2channel[make_pair(ip,port)];
                    string &writemsg = channel->getWritemsg();
                    
                    d["operate"].SetString("reply");
                    d.AddMember("train_num", 0, d.GetAllocator());
                    d["result"].SetString("");
                    StringBuffer s;
                    Writer<StringBuffer> writer(s);
                    d.Accept(writer);

                    writemsg += string(s.GetString());
                }
                
            }
            else if(operate == "query_leftnum" || operate == "buy" || operate == "pay")
            {
                string ip = d["ip"].GetString();
                int port = d["port"].GetInt();
                shared_ptr<Channel> channel = _addr2channel[make_pair(ip,port)];
                string &writemsg = channel->getWritemsg();
                
                d["operate"].SetString("reply");
                StringBuffer s;
                Writer<StringBuffer> writer(s);
                d.Accept(writer);

                writemsg += string(s.GetString());
            }
            
            msg = msg.substr(len+1);
            continue;
        }
        break;
    }
    
    return;
}

void EventLoop::handleDatabaseWrite()
{
    string &msg = _databaseChannel->getWritemsg();
    if(!msg.empty())
    {
        #ifdef DEBUG 
        printf("wrigemsg: %s\n", msg.c_str());
        #endif
        writen(_databasefd, msg);
    } 
    if(msg.size() < 1024 && !_msgQueue.empty())
    {
        string str = _msgQueue.front();
        _msgQueue.pop();
        Document d;
        d.Parse(str.c_str());
        string ip = d["ip"].GetString();
        int port = d["port"].GetInt();
        int fd = d["fd"].GetInt();
        string time = d["time"].GetString();
        string message = d["message"].GetString();
        d.Parse(message.c_str());
        d.AddMember("ip",StringRef(ip.c_str()), d.GetAllocator());
        d.AddMember("port",port, d.GetAllocator());
        d.AddMember("fd",fd, d.GetAllocator());
        d.AddMember("time", StringRef(time.c_str()), d.GetAllocator());

        string operate = d["operate"].GetString();
        if(operate == "query")
        {
            d["operate"].SetString("query_train");
            StringBuffer buffer;  
            Writer<StringBuffer> writer(buffer);  
            d.Accept(writer);
            msg += buffer.GetString();
        }
        else if(operate == "query_leftnum" || operate == "buy" || operate == "pay")
        {
            StringBuffer buffer;  
            Writer<StringBuffer> writer(buffer);  
            d.Accept(writer);
            msg += buffer.GetString();
        }
    }
    
}