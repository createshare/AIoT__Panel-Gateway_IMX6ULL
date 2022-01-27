/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-31     eason      The first version
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include<string.h>

#define DEFAULT_DB_NAME "rtsmart.db"

typedef enum
{
    DB_LOGIN = 1,
    DB_ADDUSER,
    DB_DELUSER,
    DB_CHPASS,
    DB_RESETPASS,
    DB_GETUSERINFO,
    DB_GETSYSLOG
}Type;

static sqlite3 *db = 0;
static int check_value = 0;
static int check_value1 = 0;
static char save_password[500];

static int initdb();
static int createdb();
static int adduser(char *n,char *p,char *r);
static int deluser(char *n);
static int chpass(char *n,char *op,char *np);
static int resetpass(char *n);
static int getuserinfo();
static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[]);
static int select_result_callback1(void *para, int f_num, char *f_value[], char *f_name[]);
static int select_result_callback2(void *para, int f_num, char *f_value[], char *f_name[]);
static int insert_log(char *t,char *l,char *opt,char *detail);

static int login(char *n,char *p)
{
    initdb();
    int rc = SQLITE_OK;
    char check_user[200];
    int value = 0;
    char *errMsg;
    char type_log[] = "normal";
    char level[] = "low";
    char operation[] = "login";
    char detail[50];
    memset(detail,0,sizeof(detail));
    sprintf(detail,"%s",n);
    sprintf(check_user,"select * from userinfo where username = '%s'",n);
    sqlite3_exec(db,check_user,select_result_callback2,NULL,&errMsg);
    sleep(1);
    if (check_value1 == 0)
    {
        printf("username does not exist\n");
        return -1;
    }
    else
    {
        check_value1 = 0;
        value = strcmp(p,save_password);
        if (value != 0)
        {
            printf("PASSWORD EORROR\n");
            return -1;
        }
        else
        {
            rc = insert_log(type_log,level,operation,detail);
            if (rc != SQLITE_OK)
            {
                return -1;
            }
            printf("SUCCESS\n");
        }
    }
    return rc;
}

static int initdb()
{
    FILE *fp;
    int rc = SQLITE_OK;

    fp = fopen("rtsmart.db","r");
    if (fp == NULL)
    {
        rc = createdb();
        if (rc != SQLITE_OK)
        {
            return -1;
        }
    }
    else
    {
        fclose(fp);
        rc = SQLITE_OK;
        rc = sqlite3_open(DEFAULT_DB_NAME,&db);
        if (rc != SQLITE_OK)
        {
            return -1;
        }
    }
    return rc;
}

static int createdb()
{
    char *errMsg1;
    char *errMsg2;
    char *errMsg3;
    int rc = SQLITE_OK;
    char create_userinfo_table[]="create table userinfo(username text, password text, role text ,ctime text);";
    char create_syslog_table[]="create table syslog(ctime text, log_type text, level text, operation text ,detail text);";
    char buff[50];
    char morendata[500];
    memset(morendata,0,sizeof(morendata));
    memset(buff,0,sizeof(buff));
    time_t timep;
    struct tm *ptime;
    time(&timep);
    ptime = localtime(&timep);
    sprintf(buff,"%d%d%d%d%d%d",(1900 + ptime->tm_year),(1 + ptime->tm_mon),ptime->tm_mday,(ptime->tm_hour),ptime->tm_min,ptime->tm_sec);
    
    rc = sqlite3_open(DEFAULT_DB_NAME,&db);
    if (rc != SQLITE_OK)
    {
        return -1;
    }

    rc = SQLITE_OK;
    rc = sqlite3_exec(db, create_userinfo_table, NULL, 0, &errMsg1);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg1);
        return -1;
    }

    rc = SQLITE_OK;
    sprintf(morendata,"insert into userinfo values('admin','21232f297a57a5a743894a0e4a801fc3','admin','%s');",buff);
    rc = sqlite3_exec(db,morendata,NULL,NULL,&errMsg2);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg2);
        return -1;
    }
    
    rc = SQLITE_OK;
    rc = sqlite3_exec(db, create_syslog_table, NULL, 0, &errMsg3);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg3);
        return -1;
    }
    return rc; 
}

static int adduser(char *n,char *p,char *r)
{
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
    sprintf(buff,"%d%d%d%d%d%d",(1900 + ptime->tm_year),(1 + ptime->tm_mon),ptime->tm_mday,(ptime->tm_hour),ptime->tm_min,ptime->tm_sec);
    memset(adduser,0,sizeof(adduser));
    memset(check_user,0,sizeof(check_user));
    initdb();

    sprintf(check_user,"select * from userinfo where username = '%s'",n);
    sqlite3_exec(db,check_user,select_result_callback1,NULL,&errMsg);
    sleep(1);
    if (check_value == 0)
    {
        sprintf(adduser,"insert into userinfo values('%s','%s','%s','%s');",n,p,r,buff);
        rc = sqlite3_exec(db,adduser,NULL,NULL,&errMsg);
        if (rc != SQLITE_OK)
        {
            return -1;
        }
        rc = insert_log(type_log,level,operation,detail);
        if(rc != SQLITE_OK)
        {
            return -1;
        }
        printf("USER ADD USCCESS\n");
    }
    else
    {
        printf("USERNAME ALREADY EXIST\n");
        return -1;
    }
    return rc;
}

static int deluser(char *n)
{
    char type_log[]="normal";
    char level[]="low";
    char operation[]="deluser";
    char detail[50];
    initdb();
    int rc = SQLITE_OK;
    char *errMsg;
    char delete[200];
    memset(detail,0,sizeof(detail));
    memset(delete,0,sizeof(delete));
    sprintf(detail,"%s",n);
    sprintf(delete,"delete from userinfo where username = '%s';",n);
    
    rc = sqlite3_exec(db,delete,NULL,NULL,&errMsg);
    if (rc != SQLITE_OK)
    {
        return -1;
    }

    rc = insert_log(type_log,level,operation,detail);
    if (rc != SQLITE_OK)
    {
        return -1;
    }
    printf("USER DELETE SUCCESS\n");
    return rc;
}

static int chpass(char *n,char *op,char *np)
{
    char type_log[] = "normal";
    char level[] = "low";
    char operation[] = "chpass";
    char detail[50];
    char check_user[200];
    int value = 0;
    initdb();
    int rc = SQLITE_OK;
    char *errMsg;
    char update[500];
    memset(check_user,0,sizeof(check_user));
    memset(detail,0,sizeof(detail));
    memset(update,0,sizeof(update));
    sprintf(detail,"%s",n);
    sprintf(check_user,"select * from userinfo where username = '%s'",n);
    sprintf(update,"update userinfo set password = '%s' where username = '%s'",np,n);

    sqlite3_exec(db,check_user,select_result_callback2,NULL,&errMsg);
    sleep(1);
    if (check_value1 == 0)
    {
        printf("USERNAME DOES NOT EXIST\n");
        return -1;
    }
    else
    {
        check_value1 = 0;
        value = strcmp(op,save_password);
        if (value != 0)
        {
            printf("old password is error\n");
            return -1;
        }
        else
        {
            rc = sqlite3_exec(db,update,NULL,NULL,&errMsg);
            if (rc != SQLITE_OK)
            {
                return -1;
            }
            rc = insert_log(type_log,level,operation,detail);
            if (rc != SQLITE_OK)
            {
                return -1;
            }
            printf("modify pass is success\n"); 
        }
    }
    return rc;
}

static int resetpass(char *n)
{
    char type_log[] = "normal";
    char level[] = "low";
    char operation[] = "resetpass";
    char detail[50];
    initdb();
    int rc = SQLITE_OK;
    char *errMsg;
    char resetpass[500];
    memset(detail,0,sizeof(detail));
    memset(resetpass,0,sizeof(resetpass));
    sprintf(detail,"%s",n);
    sprintf(resetpass,"update userinfo set password = '21218cca77804d2ba1922c33e0151105' where username = '%s'",n);
    rc = sqlite3_exec(db,resetpass,NULL,NULL,&errMsg);
    if (rc != SQLITE_OK)
    {
        return -1;
    }
    rc = insert_log(type_log,level,operation,detail);
    if (rc != SQLITE_OK)
    {
        return -1;
    }
    printf("PASSWORD RESET SUCCESS\n");
    return rc;
}

static int insert_log(char *t,char *l,char *opt,char *detail)
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
    sprintf(buff,"%d%d%d%d%d%d",(1900 + ptime->tm_year),(1 + ptime->tm_mon),ptime->tm_mday,(ptime->tm_hour),ptime->tm_min,ptime->tm_sec);
    memset(addlog,0,sizeof(addlog));
    sqlite3_close(db);
    initdb();
    sprintf(addlog,"insert into syslog values('%s','%s','%s','%s','%s');",buff,t,l,opt,detail);
    rc = sqlite3_exec(db,addlog,NULL,NULL,&errMsg);
    if (rc != SQLITE_OK)
    {
        return -1;
    }
    return rc; 
}

static int getuserinfo()
{
    initdb();
    int rc = SQLITE_OK;
    char *errMsg;
    char select[]="select * from userinfo;";
    if (sqlite3_exec(db,select,select_result_callback,NULL,&errMsg)!=0)
    {
        return -1;
    }
    return rc;
}

static int getsyslog()
{
    initdb();
    int rc = SQLITE_OK;
    char *errMsg;
    char select[]="select * from syslog;";
    if (sqlite3_exec(db,select,select_result_callback,NULL,&errMsg)!=0)
    {
        return -1;
    }
    return rc;
}

static int select_result_callback2(void *para, int f_num, char *f_value[], char *f_name[])
{
    int length = 0;
    length = strlen(f_value[1]);
    check_value1 = f_num;
    memset(save_password,0,sizeof(save_password)); 
    if (check_value1 != 0)
    {
        memcpy(save_password,f_value[1],strlen(f_value[1]));
        save_password[length] = '\0';
    }
    return f_num;
}

static int select_result_callback1(void *para, int f_num, char *f_value[], char *f_name[])
{
    check_value = f_num;
    return f_num;
}

static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[])
{
    int i = 0;
    for (i=0;i<f_num;i++)
    {
        printf("%s:\t%s\n",f_name[i],f_value[i]);
    }
    printf("--------------------------\n");
    return 0;
}

void usage(char * proc_name)
{
	printf("\nUsage: %s option\n", proc_name);
	printf(	"\toption:\n"
		"\t--login\n"
		"\t--adduser\n"
		"\t--deluser\n"
		"\t--chpass\n"
		"\t--resetpass\n"
		"\t--getuserinfo\n"
		"\t--getsyslog\n");
}

int main(int argc, char **argv)
{
	int rc = SQLITE_OK;
	Type testType = 0;
        if (argc == 1)
        {
                usage(argv[0]);
                return 1;
        }
	if (0 == rt_strcmp("--login",argv[1]))
	{
		testType = DB_LOGIN;
	}
	else if (0 == rt_strcmp("--adduser",argv[1]))
	{
		testType = DB_ADDUSER;
	}
	else if (0 == rt_strcmp("--deluser",argv[1]))
	{
		testType = DB_DELUSER;
	}
	else if (0 == rt_strcmp("--chpass",argv[1]))
	{
		testType = DB_CHPASS;
	}
	else if (0 == rt_strcmp("--resetpass",argv[1]))
	{
		testType = DB_RESETPASS;
	}
	else if (0 == rt_strcmp("--getuserinfo",argv[1]))
	{
		testType = DB_GETUSERINFO;
	}
	else if (0 == rt_strcmp("--getsyslog",argv[1]))
	{
		testType = DB_GETSYSLOG;
	}

	switch(testType)
	{
		case DB_LOGIN:
                        if (argc != 4)
                        {
                            printf("Usage: function name password\n");
                            return 0;
                        }
                        rc = login(argv[2],argv[3]);
			if (rc == SQLITE_OK)
			{
				printf("test login success\n");
			}
			else
			{
				printf("test login Failed\n");
			}
		break;
		case DB_ADDUSER:
                        if (argc != 5)
                        {
                            printf("Usage: function name password role\n");
                            return 0;
                        }
                        rc = adduser(argv[2],argv[3],argv[4]);
			if (rc == SQLITE_OK)
			{
				printf("test adduser table Sucess\n");
			}
			else
			{
				printf("test adduser table Failed\n");
			}
		break;
		case DB_DELUSER:
                        if (argc != 3)
                        {
                            printf("Usage: function username\n");
                            return 0;
                        }
                        rc = deluser(argv[2]);
			if (rc == SQLITE_OK)
			{
				printf("test deluser record Sucess\n");
			}
			else
			{
				printf("test deluser record Failed\n");
			}
		break;
		case DB_CHPASS:
                        if (argc != 5)
                        {
                            printf("Usage: function username oldpasswd newpassword\n");
                            return 0;
                        }
			rc = chpass(argv[2],argv[3],argv[4]);
			if (rc == SQLITE_OK)
			{
				printf("test chpass table Sucess\n");
			}
			else
			{
				printf("test chpass table Failed\n");
			}
		break;
		case DB_RESETPASS:
                        if (argc != 3)
                        {
                            printf("Usage: function username\n");
                            return 0;
                        }
			rc = resetpass(argv[2]);
			if(rc == SQLITE_OK)
			{
				printf("test resetpass  Sucess\n");
			}
			else
			{
				printf("test resetpass  Failed\n");
			}
		break;
		case DB_GETUSERINFO:
                        if (argc != 2)
                        {
                            printf("Usage: function\n");
                            return 0;
                        }
			rc = getuserinfo();
			if (rc == SQLITE_OK)
			{
				printf("test getuserinfo  Sucess\n");
			}
			else
			{
				printf("test getuserinfo  Failed\n");
			}
		break;
		case DB_GETSYSLOG:
                        if (argc != 2)
                        {
                            printf("Usage: function\n");
                            return 0;
                        }
			rc = getsyslog();
			if (rc == SQLITE_OK)
			{
				printf("test getsyslog  Sucess\n");
			}
			else
			{
				printf("test getsyslog  Failed\n");
			}
		break;
		default:
			usage(argv[0]);
			return 1;
		break;

	}

	sqlite3_close(db);
	return 0;
}
