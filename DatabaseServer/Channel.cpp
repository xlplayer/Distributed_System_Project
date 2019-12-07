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
#include <mysql/mysql.h>
#include <iostream>
using namespace std;
using std::queue;
using std::bind;
using namespace rapidjson;

void getTrains(string &start, string &end, vector<string> &trains);
void getLeftNums(string &train, string &date, string &start, string &end, string &result);
void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result);

Channel::Channel(shared_ptr<EventLoop> eventLoop, int fd)
:_eventLoop(eventLoop)
,_epoll(eventLoop->getEpoll())
,_msgQueue(eventLoop->getQueue())
,_fd(fd)
,_state(CONNECTIING)
,_curlyCount(0)
{ 
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
        if(zero) //got EOF
        {
            printf("client closed\n");
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
                    getTrains(start, end, trains);

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
                    getLeftNums(train, date, start, end, result);

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
                    buyTicket(date, train_number, start, end, passenger_name, passenger_id, result);
                    
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
                    }
                    StringBuffer s;
                    Writer<StringBuffer> writer(s);
                    d.Accept(writer);
                    _writemsg += s.GetString();
                }
                else if(operate == "pay")
                {

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
            printf("writemsg: %s\n",_writemsg.c_str());
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

void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result)
{
    string start_idx, end_idx;
    bool success = true;
    string id, seat_number, compartment_number;
    string head, tail;
    char query[1024];
    MYSQL *conn;
    MYSQL_FIELD *field;
    MYSQL_RES *res_ptr;
    MYSQL_ROW result_row;

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, CLIENT_FOUND_ROWS);
    mysql_query(conn, "set names utf8");

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train_number.c_str(), start.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    start_idx = mysql_fetch_row(res_ptr)[0];

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train_number.c_str(), end.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    end_idx = mysql_fetch_row(res_ptr)[0];

    mysql_autocommit(conn, false);
    do{
        sprintf(query, "select * from %s_%s where start <= %s and end >= %s and status = \"unsold\" limit 1 for update", date.c_str(),train_number.c_str(),start_idx.c_str(), end_idx.c_str());
        cout<<query<<endl;
        if(mysql_query(conn, query))
        { 
            success = false;
            break;
        } 
        res_ptr = mysql_store_result(conn);
        result_row = mysql_fetch_row(res_ptr);
        id = result_row[0];
        head = result_row[2];
        tail = result_row[3];
        seat_number = result_row[4];
        compartment_number = result_row[5];

        sprintf(query, "update %s_%s set status = \"unpaid\", start = %s, end = %s, passenger_name = \"%s\", passenger_id = \"%s\" \
                        where id = %s", date.c_str(),train_number.c_str(), start_idx.c_str(), end_idx.c_str(), passenger_name.c_str(), passenger_id.c_str(), id.c_str());
        cout<<query<<endl;
        if (mysql_query(conn, query)) 
        { 
            success = false;
            break;
        }

        if(atoi(start_idx.c_str()) > atoi(head.c_str()))
        {
            sprintf(query, "insert into %s_%s (status,start,end,seat_number,compartment_number) values (\"unsold\",%s,%s,%s,%s)", 
                            date.c_str(),train_number.c_str(),head.c_str(), start_idx.c_str(), seat_number.c_str(), compartment_number.c_str());
            cout<<query<<endl;
            if (mysql_query(conn, query)) 
            { 
                success = false;
                break;
            }  
        }

        if(atoi(end_idx.c_str()) < atoi(tail.c_str()))
        {
            sprintf(query, "insert into %s_%s (status,start,end,seat_number,compartment_number) values (\"unsold\",%s,%s,%s,%s)", 
                            date.c_str(),train_number.c_str(),end_idx.c_str(), tail.c_str(), seat_number.c_str(), compartment_number.c_str());
            cout<<query<<endl;
            if (mysql_query(conn, query)) 
            { 
                success = false;
                break;
            }  
        }
        
    }while(false);

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    if(success == false)
    {
        mysql_rollback(conn);
        mysql_autocommit(conn, true);
        writer.Key("result");
        writer.String("failure");
    }
    else
    {
        mysql_commit(conn);
        mysql_autocommit(conn, true);
        writer.Key("result");
        writer.String("success");
        // writer.Key("train_number");
        // writer.String(train_number.c_str());
        // writer.Key("start");
        // writer.String(start.c_str());
        // writer.Key("end");
        // writer.String(end.c_str());
        writer.Key("compartment_number");
        writer.String(compartment_number.c_str());
        writer.Key("seat_number");
        writer.String(seat_number.c_str());
    }
    writer.EndObject();
    result = s.GetString();
    
    mysql_free_result(res_ptr);
    mysql_close(conn);
    mysql_library_end();
}

void getTrains(string &start, string &end, vector<string> &trains)
{
    char query[1024];
    MYSQL *conn;
    MYSQL_FIELD *field;

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, CLIENT_FOUND_ROWS);
    mysql_query(conn, "set names utf8");
    sprintf(query, "select * from station_train where start = \"%s\" and end = \"%s\"", start.c_str(), end.c_str());
    cout<<query<<endl;
    int res = mysql_query(conn, query);
    if (res) 
	{ 
        printf("Error： mysql_query !\n");
        mysql_close(conn);
        mysql_library_end();
    }
    else 
    { 
        MYSQL_RES *res_ptr = mysql_store_result(conn);
        if (res_ptr) 
        {
            int column = mysql_num_fields(res_ptr);
            int row = mysql_num_rows(res_ptr);
            for (int i = 0; field = mysql_fetch_field(res_ptr); i++)
                printf("%s\t", field->name);
            printf("\n");
            for (int i = 0; i < row; i++)
            {
                MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
                for (int j = 0; j < column; j++)
                    printf("%s\t", result_row[j]);
                printf("\n");
                trains.push_back(result_row[3]);
            }

        }
        mysql_free_result(res_ptr);
        mysql_close(conn);
        mysql_library_end();
    }
}

void getLeftNums(string &train, string &date, string &start, string &end, string &result)
{
    string start_idx, end_idx;
    char query[1024];
    int res;
    MYSQL *conn;
    MYSQL_FIELD *field;
    MYSQL_RES *res_ptr;

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, CLIENT_FOUND_ROWS);
    mysql_query(conn, "set names utf8");

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train.c_str(), start.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    start_idx = mysql_fetch_row(res_ptr)[0];

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train.c_str(), end.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    end_idx = mysql_fetch_row(res_ptr)[0];

    sprintf(query, "select count(*) from %s_%s where start <= %s and end >= %s and status = \"unsold\"", date.c_str(), train.c_str(), start_idx.c_str(), end_idx.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    string left_num = mysql_fetch_row(res_ptr)[0];

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("train_number");
    writer.String(train.c_str());
    writer.Key("start");
    writer.String(start.c_str());
    writer.Key("end");
    writer.String(end.c_str());
    writer.Key("num");
    writer.Uint(atoi(left_num.c_str()));
    writer.EndObject();
    result = s.GetString();

    mysql_free_result(res_ptr);
    mysql_close(conn);
    mysql_library_end();
}