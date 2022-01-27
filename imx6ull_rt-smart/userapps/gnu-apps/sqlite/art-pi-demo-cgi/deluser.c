#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

static sqlite3* deluser(char *n);

static sqlite3* deluser(char *n)
{
    sqlite3 *db = NULL;
    char type_log[] = "normal";
    char level[] = "low";
    char operation[] = "deluser";
    char detail[50];
    int value = 0;
    int rc = SQLITE_OK;
    char *errMsg;
    char delete[200];
    memset(detail,0,sizeof(detail));
    memset(delete,0,sizeof(delete));
    sprintf(detail,"%s",n);
    sprintf(delete,"delete from userinfo where username = '%s';",n);
    db = initdb();
    if(db == NULL)
    {
        return NULL;
    }

    value = strcmp(n,"admin");
    if (value == 0)
    {
        printf("ename");
        return NULL;
    }
    else
    {
        rc = sqlite3_exec(db,delete,NULL,NULL,&errMsg);
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
    return db;
}

int main(void)
{
    sqlite3 *db = NULL;
    char *data;
    char n[20] = {0}; /* username */

    printf("Content-Type:text/html\n\n");
    data = getenv("QUERY_STRING");
    sscanf(data,"n=%s", n);
    db = deluser(n);

    sqlite3_close(db);
    return 0;
}
