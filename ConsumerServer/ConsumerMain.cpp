#include "MessageConsumer.h"
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
void getTrains(string &start, string &end, vector<string> &trains);
void getLeftNums(string &train, string &date, string &start, string &end, vector<string> &results);
void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result);

int main(int argc, char **argv)
{
    vector<sockaddr_in> queueServerAddr;
    initQueueServerAddr(queueServerAddr);
    shared_ptr<MessageConsumer> messageConsumer(new MessageConsumer(queueServerAddr));
    while(1)
    {
        string msg = messageConsumer->pop();
        if(msg == "pop failure")
        {
            sleep(0.1);
        }
        else
        {
            cout<<msg<<endl;
            Document d;
            d.Parse(msg.c_str());
            string ip = d["ip"].GetString();
            int port = d["port"].GetInt();
            int fd = d["fd"].GetInt();
            string message = d["message"].GetString();
            d.Parse(message.c_str());
            string operate = d["operate"].GetString();
            if(operate == "query")
            {
                string date,start,end;
                date = d["date"].GetString();
                start = d["start"].GetString();
                end = d["end"].GetString();
                vector<string> trains;
                getTrains(start, end, trains);
                vector<string> results;
                for(size_t i=0; i<trains.size(); i++)
                    getLeftNums(trains[i], date, start, end, results); 
            
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in servaddr;
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(port);
                inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
                connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

                StringBuffer s;
                Writer<StringBuffer> writer(s);
                writer.StartObject();
                writer.Key("operate");
                writer.String("reply");
                writer.Key("fd");
                writer.Int(fd);
                writer.Key("result");
                writer.StartArray();
                for(size_t i = 0; i<results.size();i++)
                    writer.String(results[i].c_str());
                writer.EndArray();
                writer.EndObject();
                write(sockfd, s.GetString(), s.GetSize());

                close(sockfd);

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
                buyTicket(date, train_number, start, end, passenger_name, passenger_id, result);
                
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in servaddr;
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(port);
                inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
                connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

                StringBuffer s;
                Writer<StringBuffer> writer(s);
                writer.StartObject();
                writer.Key("operate");
                writer.String("reply");
                writer.Key("fd");
                writer.Int(fd);
                writer.Key("result");
                writer.String(result.c_str());
                writer.EndObject();
                write(sockfd, s.GetString(), s.GetSize());

                close(sockfd);
            }
            else if(operate == "pay")
            {

            }

        }
        
    }

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
void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(60000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("operate");
    writer.String("buy");
    writer.Key("date");
    writer.String(date.c_str());
    writer.Key("train_number");
    writer.String(train_number.c_str());
    writer.Key("start");
    writer.String(start.c_str());
    writer.Key("end");
    writer.String(end.c_str());
    writer.Key("passenger_name");
    writer.String(passenger_name.c_str());
    writer.Key("passenger_id");
    writer.String(passenger_id.c_str());
    writer.EndObject();
    write(sockfd, s.GetString(), s.GetSize());

    char buf[1024];
    int n = read(sockfd, buf, 1024);
    cout<<string(buf, buf+n)<<endl;
    result = string(buf, buf+n);
    close(sockfd);
}

void getTrains(string &start, string &end, vector<string> &trains)
{

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(60000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("operate");
    writer.String("query_train");
    writer.Key("start");
    writer.String(start.c_str());
    writer.Key("end");
    writer.String(end.c_str());
    writer.EndObject();
    write(sockfd, s.GetString(), s.GetSize());

    char buf[1024];
    int n = read(sockfd, buf, 1024);
    cout<<string(buf, buf+n)<<endl;
    Document d;
    d.Parse(string(buf, buf+n).c_str());
    Value &val = d["result"];
    for(size_t i=0; i<val.Size();i++)
    {
        Value &v = val[i];
        trains.push_back(v.GetString());
    }
    for(int i=0;i<trains.size();i++) cout<<trains[i]<<" ";
    cout<<endl;
    close(sockfd);
}

void getLeftNums(string &train, string &date, string &start, string &end, vector<string> &results)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(60000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("operate");
    writer.String("query_leftnum");
    writer.Key("train");
    writer.String(train.c_str());
    writer.Key("date");
    writer.String(date.c_str());
    writer.Key("start");
    writer.String(start.c_str());
    writer.Key("end");
    writer.String(end.c_str());
    writer.EndObject();
    write(sockfd, s.GetString(), s.GetSize());

    char buf[1024];
    int n = read(sockfd, buf, 1024);
    Document d;
    d.Parse(string(buf, buf+n).c_str());
    results.push_back(d["result"].GetString());
    close(sockfd);

}