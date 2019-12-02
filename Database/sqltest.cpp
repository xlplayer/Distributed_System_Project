#include<mysql/mysql.h>
#include<stdio.h>
int main(int argc,char **argv)
{
    MYSQL *conn; /*这是一个数据库连接*/
    int res; /*执行sql語句后的返回标志*/
    MYSQL_RES *res_ptr; /*指向查询结果的指针*/
    MYSQL_FIELD *field; /*字段结构指针*/
    MYSQL_ROW result_row; /*按行返回的查询信息*/
 
    int row, column; /*查询返回的行数和列数*/
    int i, j; /*只是控制循环的两个变量*/
 
    /*初始化mysql连接my_connection*/
    conn = mysql_init(NULL);
 
    /*这里就是用了mysql.h里的一个函数，用我们之前定义的那些宏建立mysql连接，并
    返回一个值，返回不为空证明连接是成功的*/
    if (mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, CLIENT_FOUND_ROWS)) 
	{
		/*连接成功*/
        printf("数据库查询query_sql连接成功！\n");
        /*这句话是设置查询编码为utf8，这样支持中文*/
        mysql_query(conn, "set names utf8");
		
        /*下面这句话就是用mysql_query函数来执行我们刚刚传入的sql語句，
        这会返回一个int值，如果为0，证明語句执行成功*/
        res = mysql_query(conn, "select * from 20191201_G102 where seat_number = 1");
 
        if (res) 
		{ /*现在就代表执行失败了*/
            printf("Error： mysql_query !\n");
            /*不要忘了关闭连接*/
            mysql_close(conn);
            mysql_library_end();
        }
		else 
		{ 
			/*现在就代表执行成功了*/
            /*将查询的結果给res_ptr*/
            res_ptr = mysql_store_result(conn);
 
            /*如果结果不为空，就把结果print*/
            if (res_ptr) 
			{
                /*取得結果的行数和*/
                column = mysql_num_fields(res_ptr);
                row = mysql_num_rows(res_ptr);
                printf("查询到 %d 行\n", row);
 
                /*输出結果的字段名*/
                for (i = 0; field = mysql_fetch_field(res_ptr); i++)
                    printf("%s\t", field->name);
                printf("\n");
 
                /*按行输出結果*/
                for (i = 0; i < row; i++)
				{
                    result_row = mysql_fetch_row(res_ptr);
                    for (j = 0; j < column; j++)
                        printf("%s\t", result_row[j]);
                    printf("\n");
                }
 
            }
 
            /*不要忘了关闭连接*/
            mysql_free_result(res_ptr);
            mysql_close(conn);
            mysql_library_end();
        }
    }
}
