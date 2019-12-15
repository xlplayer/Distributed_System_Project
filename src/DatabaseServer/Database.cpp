#include "Database.h"
#include "base/rapidjson/stringbuffer.h"
#include "base/rapidjson/document.h"
#include "base/rapidjson/writer.h"
#include <stdlib.h>
#include "TicketManager.h"
#include <memory>
using std::shared_ptr;
using namespace rapidjson;

extern map<string, vector<Ticket> > tickets;

Database::Database()
{
    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, 0);
    mysql_query(conn, "set names utf8");
}

Database::~Database()
{
    mysql_close(conn);
    mysql_library_end();
}

void Database::getTrains(string &start, string &end, vector<string> &trains)
{
    char query[1024];

    sprintf(query, "select * from station_train where start = \"%s\" and end = \"%s\"", start.c_str(), end.c_str());
   
    int res = mysql_query(conn, query);
    if (res) 
	{ 
        perror("Error： mysql_query !\n");
    }
    else 
    { 
        res_ptr = mysql_store_result(conn);
        if (res_ptr) 
        {
            int column = mysql_num_fields(res_ptr);
            int row = mysql_num_rows(res_ptr);
            #ifdef DEBUG
            for (int i = 0; field = mysql_fetch_field(res_ptr); i++)
                printf("%s\t", field->name);
            printf("\n");
            #endif
            for (int i = 0; i < row; i++)
            {
                result_row = mysql_fetch_row(res_ptr);
                #ifdef DEBUG
                for (int j = 0; j < column; j++)
                    printf("%s\t", result_row[j]);
                printf("\n");
                #endif
                trains.push_back(result_row[3]);
            }

        }
        mysql_free_result(res_ptr);

    }
}

void Database::getLeftNums(string &train, string &date, string &start, string &end, string &result)
{
    string start_idx, end_idx;
    char query[1024];
    int res;

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train.c_str(), start.c_str());
    #ifdef DEBUG 
    printf("%s\n",query);
    #endif
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    start_idx = mysql_fetch_row(res_ptr)[0];

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train.c_str(), end.c_str());
    #ifdef DEBUG 
    printf("%s\n",query);
    #endif
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    end_idx = mysql_fetch_row(res_ptr)[0];

    int left_num = 0;
    vector<Ticket> &tik = tickets[date+"_"+train];
    int size = tik.size();
    for(int i=0;i<size;i++)
    {
        Ticket &item = tik[i];
        //MutexLockGuard lock(item._mutex);
        if(item._valid)
        {
            unsigned  bits = 0;
            for(int i = atoi(start_idx.c_str()); i < atoi(end_idx.c_str()); i++)
                bits |= (1<<(i-1));
            if(bits & item._bits) left_num++;
        }  
    }

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
    writer.Uint(left_num);
    writer.EndObject();
    result = s.GetString();

    mysql_free_result(res_ptr);
}


void Database::buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result)
{
    string start_idx, end_idx;
    bool success = true;
    string id, seat_number, compartment_number;
    string head, tail;
    char query[1024];

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train_number.c_str(), start.c_str());
    #ifdef DEBUG 
    printf("%s\n",query);
    #endif
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    start_idx = mysql_fetch_row(res_ptr)[0];
    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train_number.c_str(), end.c_str());
    #ifdef DEBUG 
    printf("%s\n",query);
    #endif
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    end_idx = mysql_fetch_row(res_ptr)[0];


    unsigned  bits = 0;
    for(int i = atoi(start_idx.c_str()); i < atoi(end_idx.c_str()); i++)
        bits |= (1<<(i-1));
        
    Ticket old_ticket;
    vector<Ticket> &tik = tickets[date+"_"+train_number];
    int size = tik.size();
    for(int i=0;i<size;i++)
    {
        Ticket &item = tik[i];
        if(item._valid && (item._bits & bits))//double check
        {
            MutexLockGuard lock(item._mutex);
            if(item._valid && (item._bits & bits))//double check
            {
                item._valid = false;
                sprintf(query, "select * from %s_%s where id = \"%s\" for update", date.c_str(),train_number.c_str(),item._id.c_str());
                #ifdef DEBUG 
                printf("%s\n",query);
                #endif

                old_ticket._id = item._id;
                old_ticket._bits = item._bits;//don't copy mutex!
                break;
            }
        }
    }
    if(old_ticket._id == "null")//no ticket left
    {
        result = "{\"result\":\"failure\"}";
        return ;
    }

    int id_x=-1,id_y=-1;
    mysql_autocommit(conn, false);
    do{
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
        #ifdef DEBUG 
        printf("%s\n",query);
        #endif
        if (mysql_query(conn, query)) 
        { 
            success = false;
            break;
        }

        if(atoi(start_idx.c_str()) > atoi(head.c_str()))
        {
            sprintf(query, "insert into %s_%s (status,start,end,seat_number,compartment_number) values (\"unsold\",%s,%s,%s,%s)", 
                            date.c_str(),train_number.c_str(),head.c_str(), start_idx.c_str(), seat_number.c_str(), compartment_number.c_str());
            #ifdef DEBUG 
            printf("%s\n",query);
            #endif
            if (mysql_query(conn, query)) 
            { 
                success = false;
                break;
            }  
            id_x = mysql_insert_id(conn);
        }

        if(atoi(end_idx.c_str()) < atoi(tail.c_str()))
        {
            sprintf(query, "insert into %s_%s (status,start,end,seat_number,compartment_number) values (\"unsold\",%s,%s,%s,%s)", 
                            date.c_str(),train_number.c_str(),end_idx.c_str(), tail.c_str(), seat_number.c_str(), compartment_number.c_str());
            #ifdef DEBUG 
            printf("%s\n",query);
            #endif
            if (mysql_query(conn, query)) 
            { 
                success = false;
                break;
            }  
            id_y = mysql_insert_id(conn);
        }
        
    }while(false);

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    if(success == false)
    {
        tickets[date+"_"+train_number].push_back(old_ticket);
        mysql_rollback(conn);
        mysql_autocommit(conn, true);
        writer.Key("result");
        writer.String("failure");
    }
    else
    {  
        mysql_commit(conn);
        mysql_autocommit(conn, true);
        char buf[20];
        if(id_x!=-1) 
        {
            sprintf(buf,"%d",id_x);
            tickets[date+"_"+train_number].push_back(Ticket(head,start_idx,buf));
        }
        if(id_y!=-1) 
        {
            sprintf(buf,"%d",id_y);
            tickets[date+"_"+train_number].push_back(Ticket(end_idx,tail,buf));
        }
        writer.Key("result");
        writer.String("success");
        writer.Key("compartment_number");
        writer.String(compartment_number.c_str());
        writer.Key("seat_number");
        writer.String(seat_number.c_str());
        writer.Key("id");
        writer.String(id.c_str());
    }
    writer.EndObject();
    result = s.GetString();

    mysql_free_result(res_ptr);
}

void Database::payTicket(string &date, string &train_number, string &id, string &result)
{
    char query[1024];
    do
    {
        sprintf(query, "select status from %s_%s where id = \"%s\" for update", date.c_str(),train_number.c_str(),id.c_str());
        #ifdef DEBUG 
        printf("%s\n",query);
        #endif
        if (mysql_query(conn, query)) 
        { 
            result = "付款失败!";
            break;
        }

        res_ptr = mysql_store_result(conn);
        string status = mysql_fetch_row(res_ptr)[0];
        if(status == "unsold")
            result = "错误！该车票未被购买！";
        else if(status == "unpaid")
        {
            sprintf(query, "update %s_%s set status = \"paid\" where id = %s", date.c_str(),train_number.c_str(), id.c_str());
            #ifdef DEBUG 
            printf("%s\n",query);
            #endif
            if (mysql_query(conn, query))  
                result = "付款失败!";
            else 
                result = "付款成功!";
        }
        else if(status == "paid")
            result = "错误！该车票已付款！";
        
    } while (false);
    
    mysql_free_result(res_ptr);
}