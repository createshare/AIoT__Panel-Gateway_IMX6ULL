#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

static sqlite3* adduser(char *n,char *p,char *r);
static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[]);

static int check_value = 0;

static sqlite3* adduser(char *n,char *p,char *r)
{
    sqlite3 *db = NULL;
    int rc = SQLITE_OK;
    char adduser[500];
    char *errMsg;
    char buff[50];
    char type_log[] = "normal";
    char level[] = "low";
    char operation[] = "adduser";
    char detail[50];
    char check_user[200];
    time_t timep;
    struct tm *ptime;
    memset(detail,0,sizeof(detail));
    memset(buff,0,sizeof(buff));
    time(&timep);
    ptime = localtime(&timep);
    sprintf(detail,"%s",n);
    conversion_time(ptime,buff);
    memset(adduser,0,sizeof(adduser));
    memset(check_user,0,sizeof(check_user));
    db = initdb();
    if(db == NULL)
    {
        return NULL;
    }

    sprintf(check_user,"select * from userinfo where username = '%s'",n);
    sqlite3_exec(db,check_user,select_result_callback,NULL,&errMsg);
    sleep(1);
    if (check_value == 0)
    {
        sprintf(adduser,"insert into userinfo values('%s','%s','%s','%s');",n,p,r,buff);
        rc = sqlite3_exec(db,adduser,NULL,NULL,&errMsg);
        if (rc != SQLITE_OK)
        {
            return NULL;
        }
        rc = insert_log(db,type_log,level,operation,detail);
        if (rc != SQLITE_OK)
        {
            return NULL;
        }
        printf("success");
    }
    else
    {
        printf("ename");
        return NULL;
    }
    return db;
}

static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[])
{
    check_value = f_num;
    return f_num;
}

int main(void)
{
    sqlite3 *db = NULL;
    char *data;
    char n[20] = {0}; /* username */
    char p[50] = {0}; /* password, md5 string max 32 ch */
    char r[20] = {0}; /* role */

    printf("Content-Type:text/html\n\n");
    data = getenv("QUERY_STRING");
    sscanf(data,"n=%[^&]&p=%[^&]&r=%s", n, p, r);
    db = adduser(n, p, r);

    sqlite3_close(db);
    return 0;
}
