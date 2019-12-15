#ifndef DARABASE_H
#define DATABASE_H

#include <mysql/mysql.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <assert.h>
#include "TicketManager.h"

using std::vector;
using std::string;
using std::map;

class Database
{
    public:
        Database();
        ~Database();
        void getTrains(string &start, string &end, vector<string> &trains);
        void getLeftNums(string &train, string &date, string &start, string &end, string &result);
        void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result);
        void payTicket(string &date, string &train_number, string &id, string &result);
    private:
        MYSQL *conn;
        MYSQL_FIELD *field;
        MYSQL_RES *res_ptr;
        MYSQL_ROW result_row;
};
#endif