#include "EventLoop.h"
#include "Epoll.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;
using namespace rapidjson;

extern int socket_bind(int port);

EventLoop::EventLoop()
:_epoll(new Epoll())
,_messageProducer(new MessageProducer())
{
    _listenfd = socket_bind(0);
    listen(_listenfd, 100);
    _epoll->add_event(_listenfd, EPOLLIN);

    struct sockaddr_in listenaddr;
    socklen_t listenaddr_len = sizeof(listenaddr);
    getsockname(_listenfd, (struct sockaddr *)&listenaddr, &listenaddr_len);
    _listen_ip = inet_ntoa(listenaddr.sin_addr);
    _listen_port = ntohs(listenaddr.sin_port);
}

EventLoop::~EventLoop()
{}

void EventLoop::handle_read(int fd)
{
    int nread = read(fd, _buf, BUFFSIZE);
    if(nread == -1)
    {
        perror("read error");
        close(fd);
        _epoll->del_event(fd, EPOLLIN);
    }
    else if(nread == 0)
    {
        fprintf(stderr,"client close.\n");
        close(fd);
        _epoll->del_event(fd, EPOLLIN);
    }
    else
    {
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(_buf[i]);
        printf("\n");
    
        StringBuffer s;
        Writer<StringBuffer> writer(s);
        Document d;
        d.Parse(string(_buf,_buf+nread).c_str());
        string operate = d["operate"].GetString();
        if(operate == "reply")
        {
            int request_fd = d["fd"].GetInt();
            write(request_fd, _buf, nread);
        }

        else
        {
            string msg = string(_buf, _buf+nread);
            
            msg = _messageProducer->push(_listen_ip, _listen_port, fd, msg);
            if(msg == "push success")
            {
                //write(fd, "waiting result...", 17);
            }
            else if(msg == "push failure")
            {
                s.Clear();
                writer.StartObject();
                writer.Key("result");writer.String("failure");
                writer.EndObject();
                write(fd, s.GetString(), s.GetSize());
            }
        }            
    }    
}

void EventLoop::hadnle_accept(int listenfd)
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
    if(clientfd == -1)
    {
        perror("accept error");
        exit(1);
    }
    _epoll->add_event(clientfd, EPOLLIN);
}

void EventLoop::loop()
{
    while(1)
    {
        _epoll->wait_event();
        for(int i = 0; i < _epoll->_eventNum; i++)
        {
            int fd = _epoll->_events[i].data.fd;
            if(fd == _listenfd && _epoll->_events[i].events & EPOLLIN)
                hadnle_accept(_listenfd);
            else if(_epoll->_events[i].events & EPOLLIN)
                handle_read(fd);
        }
    }
}

void EventLoop::addToEpoll(int fd){
    _epoll->add_event(fd, EPOLLIN);
}