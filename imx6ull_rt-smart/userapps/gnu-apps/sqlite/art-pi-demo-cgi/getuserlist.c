#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

static sqlite3* getuserinfo();
static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[]);

static sqlite3* getuserinfo()
{
    sqlite3 *db = NULL;
    char *errMsg;
    char select[]="select * from userinfo;";
    db = initdb();
    if(db == NULL)
    {
        return NULL;
    }
    if (sqlite3_exec(db,select,select_result_callback,NULL,&errMsg)!=0)
    {
        return db;
    }
    return db;
}

static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[])
{
    int i;
    int in_order_to_print = f_num - 1;
    for (i=0;i<f_num;i++)
    {
        if (i == in_order_to_print)
        {
            printf("%s",f_value[i]);
        }
        else
        {
            printf("%s;",f_value[i]);
        }
    }
    printf("&");
    return 0;
}

int main(void)
{
    sqlite3* db = NULL;

    printf("Content-Type:text/html\n\n");
    db = getuserinfo();
    sqlite3_close(db);

    return 0;
}
