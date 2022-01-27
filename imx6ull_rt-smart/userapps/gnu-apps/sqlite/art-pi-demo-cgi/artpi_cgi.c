#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

#define DEFAULT_DB_NAME "rtsmart.db"

void conversion_time(struct tm *ptime,char *buff)
{
    char year_buff[20];
    char month_buff[20];
    char day_buff[20];
    char hour_buff[20];
    char minute_buff[20];
    char sec_buff[20];
    sprintf(year_buff, "%d", (1900 + ptime->tm_year));
    if ((1 + ptime->tm_mon) < 10) 
    {   
	sprintf(month_buff, "%d", 0);
        sprintf(month_buff+1, "%d", (1 + ptime->tm_mon));
    }   
    else
    {   
        sprintf(month_buff, "%d", (1 + ptime->tm_mon));
    }   
    if (ptime->tm_mday < 10) 
    {   
        sprintf(day_buff, "%d", 0);
        sprintf(day_buff+1, "%d", ptime->tm_mday);
    }   
    else
    {   
        sprintf(day_buff, "%d", ptime->tm_mday);
    }   
    if (ptime->tm_hour < 10) 
    {   
        sprintf(hour_buff, "%d", 0);
        sprintf(hour_buff+1, "%d", ptime->tm_hour);
    }   
    else
    {   
        sprintf(hour_buff, "%d", ptime->tm_hour);
    }   
    if (ptime->tm_min < 10) 
    {   
        sprintf(minute_buff,"%d", 0);
        sprintf(minute_buff+1, "%d", ptime->tm_min);
    }   
    else
    {
        sprintf(minute_buff, "%d",ptime->tm_min);
    }
    if (ptime->tm_sec < 10)
    {
        sprintf(sec_buff, "%d", 0);
        sprintf(sec_buff+1, "%d", ptime->tm_sec);
    }
    else
    {
        sprintf(sec_buff, "%d",ptime->tm_sec);
    }
    sprintf(buff, "%s-%s-%s_%s:%s:%s", year_buff,month_buff,day_buff,hour_buff,minute_buff,sec_buff);
}

sqlite3* initdb()
{
    sqlite3 *db = NULL;
    FILE *fp;
    int rc = SQLITE_OK;

    fp = fopen(DEFAULT_DB_NAME, "r");
    if (fp == NULL)
    {
        rc = createdb(&db);
        if (rc != SQLITE_OK)
        {
            return NULL;
        }
    }
    else
    {
        fclose(fp);
        rc = SQLITE_OK;
        rc = sqlite3_open(DEFAULT_DB_NAME,&db);
        if (rc != SQLITE_OK)
        {
            return NULL;
        }
    }
    return db;
}

int createdb(sqlite3** db)
{
    char *errMsg1;
    char *errMsg2;
    char *errMsg3;
    char *errMsg4;
    char *errMsg5;
    char *errMsg6;
    char *errMsg7;
    int rc = SQLITE_OK;
    char create_userinfo_table[]="create table userinfo(username text, password text, role text ,ctime text);";
    char create_syslog_table[]="create table syslog(ctime text, log_type text, level text, operation text ,detail text);";
    char create_sysconf_table[]="create table sysconf(name text, value text);";
    char buff[50];
    char morendata[500];
    char sysconf_buff1[] = "insert into sysconf values('sysrefresh','5000');";
    char sysconf_buff2[] = "insert into sysconf values('ledrefresh','1000');";
    char sysconf_buff3[] = "insert into sysconf values('timerefresh','300');";
    memset(morendata,0,sizeof(morendata));
    memset(buff,0,sizeof(buff));
    time_t timep;
    struct tm *ptime;
    time(&timep);
    ptime = localtime(&timep);
    conversion_time(ptime,buff);
    rc = sqlite3_open(DEFAULT_DB_NAME,db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(*db);
        return -1;
    }

    rc = SQLITE_OK;
    rc = sqlite3_exec(*db, create_userinfo_table, NULL, 0, &errMsg1);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg1);
        return -1;
    }

    rc = SQLITE_OK;
    sprintf(morendata,"insert into userinfo values('admin','21232f297a57a5a743894a0e4a801fc3','admin','%s');",buff);
    rc = sqlite3_exec(*db,morendata,NULL,NULL,&errMsg2);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg2);
        return -1;
    }

    rc = SQLITE_OK;
    rc = sqlite3_exec(*db, create_syslog_table, NULL, 0, &errMsg3);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg3);
        return -1;
    }

    rc = SQLITE_OK;
    rc = sqlite3_exec(*db, create_sysconf_table, NULL, 0, &errMsg4);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg4);
        return -1;
    }
    rc = SQLITE_OK;
    rc = sqlite3_exec(*db,sysconf_buff1,NULL,NULL,&errMsg5);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg5);
        return -1;
    }
    rc = SQLITE_OK;
    rc = sqlite3_exec(*db,sysconf_buff2,NULL,NULL,&errMsg6);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg6);
        return -1;
    }
    rc = SQLITE_OK;
    rc = sqlite3_exec(*db,sysconf_buff3,NULL,NULL,&errMsg7);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg7);
        return -1;
    }
    return rc;
}

int insert_log(sqlite3 *db,char *t,char *l,char *opt,char *detail)
{
    int rc = SQLITE_OK;
    char *errMsg;
    char buff[50];
    char addlog[500];
    memset(buff,0,sizeof(buff));
    time_t timep;
    struct tm *ptime;
    time(&timep);
    ptime = localtime(&timep);
    conversion_time(ptime,buff);
    memset(addlog,0,sizeof(addlog));
    sprintf(addlog,"insert into syslog values('%s','%s','%s','%s','%s');",buff,t,l,opt,detail);
    rc = sqlite3_exec(db,addlog,NULL,NULL,&errMsg);
    if (rc != SQLITE_OK)
    {
        return -1;
    }
    return rc;
}
