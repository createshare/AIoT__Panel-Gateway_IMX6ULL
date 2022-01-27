#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

static sqlite3* dellog(char *t);

static sqlite3* dellog(char *t)
{
    sqlite3 *db = NULL;
    int rc = SQLITE_OK;
    char *errMsg;
    char delete[200];
    memset(delete,0,sizeof(delete));
    sprintf(delete,"delete from syslog where ctime = '%s';",t);
    db = initdb();
    if(db == NULL)
    {
        return NULL;
    }

    rc = sqlite3_exec(db,delete,NULL,NULL,&errMsg);
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
    char t[40] = {0}; /* time */

    printf("Content-Type:text/html\n\n");
    data = getenv("QUERY_STRING");
    sscanf(data,"t=%s", t);
    db = dellog(t);

    sqlite3_close(db);
    return 0;
}
