#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

static sqlite3* getsysconf();
static sqlite3* savesysconf(char *n,char *v);
static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[]);

static sqlite3* getsysconf()
{
    sqlite3 *db = NULL;
    char *errMsg;
    char select[]="select * from sysconf;";
    db = initdb();
    if(db == NULL)
    {
        return NULL;
    }
    if (sqlite3_exec(db,select,select_result_callback,NULL,&errMsg)!=0)
    {
        return NULL;
    }
    return db;
}

static sqlite3* savesysconf(char *n,char *v)
{
    sqlite3 *db = NULL;
    int rc = SQLITE_OK;
    int value = 0;
    int value1 = 0;
    int value2 = 0;
    char *errMsg;
    char update[500];
    memset(update,sizeof(update),'0');
    sprintf(update,"update sysconf set value = '%s' where name = '%s';",v,n);
    db = initdb();
    if(db == NULL)
    {
        printf("db is NULL\n");
        return NULL;
    }

    value = strcmp(n,"sysrefresh"); 
    value1 = strcmp(n,"ledrefresh"); 
    value2 = strcmp(n,"timerefresh"); 
    if(value != 0 && value1 != 0 && value2 != 0)
    {
        return NULL;
    }
    
    rc = sqlite3_exec(db,update,NULL,NULL,&errMsg);
    if (rc != SQLITE_OK)
    {
        return NULL;
    }
    return db;
}

static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[])
{
    int i;
    int in_order_to_print = f_num - 1;
    for (i=0;i<in_order_to_print;i++)
    {
            printf("%s",f_value[i+1]);
    }
    printf("&");
    return 0;
}

int main(void)
{
    sqlite3* db = NULL;

    char *data;
    char tp[20] = {0};
    char value[128] = {0};
    printf("Content-Type:text/html\n\n");
    data = getenv("QUERY_STRING");
    sscanf(data,"type=%[^&]&val=%s", tp, value);
    
    switch(tp[0])
    {
    case '0': /* get sysconf info */
        db = getsysconf();
	break;
    case '1': /* save sysrefresh */
        db = savesysconf("sysrefresh", value);
	break;
    case '2': /* save ledrefresh */
        db = savesysconf("ledrefresh", value);
	break;
    case '3': /* save ledrefresh */
        db = savesysconf("timerefresh", value);
	break;
    default:
	break;
    }

    sqlite3_close(db);
    return 0;
}
