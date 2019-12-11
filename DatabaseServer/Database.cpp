#include "Database.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include <iostream>
#include <stdlib.h>

using namespace rapidjson;
using namespace std;

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
    cout<<query<<endl;
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
            for (int i = 0; field = mysql_fetch_field(res_ptr); i++)
                printf("%s\t", field->name);
            printf("\n");
            for (int i = 0; i < row; i++)
            {
                result_row = mysql_fetch_row(res_ptr);
                for (int j = 0; j < column; j++)
                    printf("%s\t", result_row[j]);
                printf("\n");
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
}


void Database::buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result)
{
    string start_idx, end_idx;
    bool success = true;
    string id, seat_number, compartment_number;
    string head, tail;
    char query[1024];

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
        writer.Key("compartment_number");
        writer.String(compartment_number.c_str());
        writer.Key("seat_number");
        writer.String(seat_number.c_str());
    }
    writer.EndObject();
    result = s.GetString();
    
    mysql_free_result(res_ptr);
}