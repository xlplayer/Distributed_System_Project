#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../src/base/rapidjson/stringbuffer.h"
#include "../src/base/rapidjson/writer.h"
#include "../src/base/rapidjson/document.h"
#include <vector>
#include <iostream>
using namespace std;
using namespace rapidjson;

const int BUFFLEN = 102400;

int main(int argc, char **argv)
{
    if(argc != 3) 
    {
        perror("usage: ./client <ipaddress> <port>");
        exit(1);
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) <0)
    {
        perror("connect failed.");
        return 0;
    }

    string passenger_name, passenger_id;
    //cout<<"请输入姓名:";cin>>passenger_name;
    //cout<<"请输入身份证号:";cin>>passenger_id;
    passenger_name = "刘晓黎";
    passenger_id = "123456";
    while(1)
    {
        Document tickets[100];
        string start, end, date;
        /*cout<<"请输入日期:";cin>>date;*/
        date = "20191201";
        cout<<"请输入购票区间"<<endl;
        cout<<"出发地:";cin>>start;
        cout<<"目的地:";cin>>end;
        cout<<"开始查询..."<<endl;
        
        StringBuffer s;
        Writer<StringBuffer> writer(s);
        writer.StartObject();
        writer.Key("operate");writer.String("query");
        writer.Key("date");writer.String(date.c_str());
        writer.Key("start");writer.String(start.c_str());
        writer.Key("end"); writer.String(end.c_str());
        writer.EndObject();
        write(sockfd, s.GetString(), s.GetSize());
        char buf[BUFFLEN];
        int nread = read(sockfd, buf, BUFFLEN);
        string result = string(buf, buf+nread);
        //cout<<result<<endl;
        Document d;
        int curlyCount = 0;
        int train_num = 0;
        for(int i=0;!result.empty();i++)
        {
            Document &v = tickets[i];

            int len;
            for(len=0;len<result.length();len++) //count { and }
            {
                if(result[len]=='{') curlyCount++;
                else if(result[len]=='}') curlyCount--;
                if(curlyCount == 0) break;
            } 
            if(curlyCount == 0)
            {
                d.Parse(result.substr(0,len+1).c_str());
                if(d["train_num"].GetInt() == 0)
                {
                    cout<<"未找到符合条件的列车！"<<endl;
                    return 0;
                }
                train_num++;
                v.Parse(d["result"].GetString());
                cout<<"["<<i<<"] "<<"车次:"<<v["train_number"].GetString()<<"\t\t出发站:"<<v["start"].GetString()\
                 <<"\t到达站:"<<v["end"].GetString()<<"\t剩余票数:"<<v["num"].GetUint()<<endl;
                if(d["train_num"].GetInt() == i+1) break;  
            }

            result = result.substr(len+1);
            continue;
        }
        int id;
        while(1){
            cout<<"请选择购买的车票(输入编号):"<<endl;cin>>id;
            if(id>=0&&id<train_num) break;
            else{
                cout<<"输入错误!"<<endl;
            }
        }

        cout<<"开始尝试购买..."<<endl;
        s.Clear();
        writer.Reset(s);
        writer.StartObject();
        writer.Key("operate");writer.String("buy");
        writer.Key("date");writer.String(date.c_str());
        writer.Key("train_number");writer.String(tickets[id]["train_number"].GetString());
        writer.Key("start");writer.String(tickets[id]["start"].GetString());
        writer.Key("end"); writer.String(tickets[id]["end"].GetString());
        writer.Key("passenger_name");writer.String(passenger_name.c_str());
        writer.Key("passenger_id");writer.String(passenger_id.c_str());
        writer.EndObject();
        write(sockfd, s.GetString(), s.GetSize());
        nread = read(sockfd, buf, BUFFLEN);
        result = string(buf, buf+nread);
        d.Parse(result.c_str());
        string status = d["result"].GetString();
        if(status == "success"){
            cout<<"购票成功，以下是购票信息:"<<endl;
            cout<<"日期:"<<d["date"].GetString()<<"\t车次:"<<d["train_number"].GetString()<<"\t出发站:"<<d["start"].GetString()\
            <<"\t到达站:"<<d["end"].GetString()<<"\t车厢号:"<<d["compartment_number"].GetString()\
            <<"\t座位号:"<<d["seat_number"].GetString()<<endl;

            cout<<"是否付款(输入Y/N)):"<<endl;
            string pay;
            cin>>pay;
            if(pay=="Y"||pay=="y")
            {
                s.Clear();
                writer.Reset(s);
                writer.StartObject();
                writer.Key("operate");writer.String("pay");
                writer.Key("train_number");writer.String(d["train_number"].GetString());
                writer.Key("date");writer.String(d["date"].GetString());
                writer.Key("id");writer.String(d["id"].GetString());
                writer.EndObject();
                write(sockfd, s.GetString(), s.GetSize());
                nread = read(sockfd, buf, BUFFLEN);
                result = string(buf, buf+nread);
                d.Parse(result.c_str());
                string status = d["result"].GetString();
                cout<<status<<endl;
            }
            else
            {
                cout<<"你放弃了支付!"<<endl;
            }
        }
        else
        {
            cout<<"购票失败"<<endl;
        }
    }
}