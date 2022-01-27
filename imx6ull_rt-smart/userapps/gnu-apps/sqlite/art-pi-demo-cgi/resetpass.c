#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

static sqlite3* resetpass(char *n);

static sqlite3* resetpass(char *n)
{
    sqlite3 *db = NULL;
    char type_log[] = "normal";
    char level[] = "low";
    char operation[] = "resetpass";
    char detail[50];
    int rc = SQLITE_OK;
    char *errMsg;
    char resetpass[500];
    memset(detail,0,sizeof(detail));
    memset(resetpass,0,sizeof(resetpass));
    sprintf(detail,"%s",n);
    sprintf(resetpass,"update userinfo set password = '21218cca77804d2ba1922c33e0151105' where username = '%s'",n);
    db = initdb();
    if(db == NULL)
    {
        return NULL;
    }
    rc = sqlite3_exec(db,resetpass,NULL,NULL,&errMsg);
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
    db = resetpass(n);

    sqlite3_close(db);
    return 0;
}
