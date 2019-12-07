#include<iostream>
#include<string.h>
#include<string>
#include<stdio.h>
# include <stdlib.h>
#include<sys/time.h>
using namespace std;
#include <memory>
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
using namespace rapidjson;

int x = 0;
class A
{
    public:
        A(){};
        ~A(){x++;cout<<x<<endl;}
};

static string GetLocalTimeWithMs(void)
{
    string defaultTime = "19700101000000000";
    try
    {
        struct timeval curTime;
        gettimeofday(&curTime, NULL);
        int milli = curTime.tv_usec / 1000;

        char buffer[80] = {0};
        struct tm nowTime;
        localtime_r(&curTime.tv_sec, &nowTime);//把得到的值存入临时分配的内存中，线程安全
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &nowTime);

        char currentTime[84] = {0};
        snprintf(currentTime, sizeof(currentTime), "%s%03d", buffer, milli);

        return currentTime;
    }
    catch(const std::exception& e)
    {
        return defaultTime;
    }
    catch (...)
    {
        return defaultTime;
    }
}

int main()
{
    shared_ptr<A> p1(new A());
    shared_ptr<A> p2 = p1;
    shared_ptr<A> p3(p1);
    // struct timeval tv;
    // time_t time;
    // char str_t[26] = {0};
    // gettimeofday(&tv, NULL);
    // time = tv.tv_sec;
    // struct tm* p_time = localtime(&time);
    // strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    // cout << str_t<<endl;
    // cout<<GetLocalTimeWithMs()<<endl;

    struct timeval cur;
    gettimeofday(&cur,NULL);
    time_t time = cur.tv_sec;
    struct tm* p_time = localtime(&time);
    char s[26] = {0},t[40] = {0};
    strftime(s, 26, "%Y-%m-%d %H:%M:%S", p_time);
    snprintf(t,40,"%s:%ld",s,cur.tv_usec);
    string _time = string(t,t+strlen(t));
    cout<<_time;

    StringBuffer ss;
    Writer<StringBuffer> writer(ss);
    writer.StartObject();
    writer.Key("result");
    writer.String("aaa");
    writer.EndObject();
    cout<<ss.GetString()<<endl;

    Document d;
    d.Parse(ss.GetString());
    StringBuffer buffer;  
    Writer<StringBuffer> writerd(buffer);  
    Value array(kArrayType);
    Document::AllocatorType& allocator = d.GetAllocator();
    for (int i = 5; i <= 10; i++)
        array.PushBack(i, allocator);

    d.AddMember("array",array,d.GetAllocator());
    d.Accept(writerd);

    cout<<buffer.GetString()<<endl;
}