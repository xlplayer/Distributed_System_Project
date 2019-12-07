#include "Thread.h"
#include "EventLoop.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <iostream>
using namespace std;
using namespace rapidjson;
using std::shared_ptr;
using std::vector;

void initQueueServerAddr(vector<sockaddr_in> &queueServerAddr);
void initDatabaseServerAddr(vector<sockaddr_in> &databaseServerAddr);

int main(int argc, char **argv)
{
    vector<sockaddr_in> queueServerAddr,databaseServerAddr;
    initQueueServerAddr(queueServerAddr);
    initDatabaseServerAddr(databaseServerAddr);

    shared_ptr<EventLoop> eventLoop(new EventLoop(queueServerAddr,databaseServerAddr));
    //eventLoop->loop();
    shared_ptr<Thread> msgThread(new Thread(bind(&EventLoop::loop, eventLoop)));
    printf("zzz\n");
    msgThread->start();
    while(1);

}

void initQueueServerAddr(vector<sockaddr_in> &queueServerAddr)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(50000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    queueServerAddr.push_back(servaddr);
}

void initDatabaseServerAddr(vector<sockaddr_in> &databaseServerAddr)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(60000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    databaseServerAddr.push_back(servaddr);
}

// void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result)
// {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     struct sockaddr_in servaddr;
//     bzero(&servaddr, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons(60000);
//     inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
//     connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

//     StringBuffer s;
//     Writer<StringBuffer> writer(s);
//     writer.StartObject();
//     writer.Key("operate");
//     writer.String("buy");
//     writer.Key("date");
//     writer.String(date.c_str());
//     writer.Key("train_number");
//     writer.String(train_number.c_str());
//     writer.Key("start");
//     writer.String(start.c_str());
//     writer.Key("end");
//     writer.String(end.c_str());
//     writer.Key("passenger_name");
//     writer.String(passenger_name.c_str());
//     writer.Key("passenger_id");
//     writer.String(passenger_id.c_str());
//     writer.EndObject();
//     write(sockfd, s.GetString(), s.GetSize());

//     char buf[1024];
//     int n = read(sockfd, buf, 1024);
//     cout<<string(buf, buf+n)<<endl;
//     result = string(buf, buf+n);
//     close(sockfd);
// }

// void getTrains(string &start, string &end, vector<string> &trains)
// {

//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     struct sockaddr_in servaddr;
//     bzero(&servaddr, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons(60000);
//     inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
//     connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

//     StringBuffer s;
//     Writer<StringBuffer> writer(s);
//     writer.StartObject();
//     writer.Key("operate");
//     writer.String("query_train");
//     writer.Key("start");
//     writer.String(start.c_str());
//     writer.Key("end");
//     writer.String(end.c_str());
//     writer.EndObject();
//     write(sockfd, s.GetString(), s.GetSize());

//     char buf[1024];
//     int n = read(sockfd, buf, 1024);
//     cout<<string(buf, buf+n)<<endl;
//     Document d;
//     d.Parse(string(buf, buf+n).c_str());
//     Value &val = d["result"];
//     for(size_t i=0; i<val.Size();i++)
//     {
//         Value &v = val[i];
//         trains.push_back(v.GetString());
//     }
//     for(int i=0;i<trains.size();i++) cout<<trains[i]<<" ";
//     cout<<endl;
//     close(sockfd);
// }

// void getLeftNums(string &train, string &date, string &start, string &end, vector<string> &results)
// {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     struct sockaddr_in servaddr;
//     bzero(&servaddr, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons(60000);
//     inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
//     connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

//     StringBuffer s;
//     Writer<StringBuffer> writer(s);
//     writer.StartObject();
//     writer.Key("operate");
//     writer.String("query_leftnum");
//     writer.Key("train");
//     writer.String(train.c_str());
//     writer.Key("date");
//     writer.String(date.c_str());
//     writer.Key("start");
//     writer.String(start.c_str());
//     writer.Key("end");
//     writer.String(end.c_str());
//     writer.EndObject();
//     write(sockfd, s.GetString(), s.GetSize());

//     char buf[1024];
//     int n = read(sockfd, buf, 1024);
//     Document d;
//     d.Parse(string(buf, buf+n).c_str());
//     results.push_back(d["result"].GetString());
//     close(sockfd);

// }