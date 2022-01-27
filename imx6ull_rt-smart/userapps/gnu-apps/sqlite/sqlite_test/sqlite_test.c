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

#define DEFAULT_DB_NAME "employee.db"

typedef enum
{
	DB_OPEN = 1,
	DB_CREATE_TABLE,
	DB_INSERT,
	DB_DELETE,
	DB_UPDATE,
	DB_SELECT
}Type;

static sqlite3 *db = 0;

static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[])
{
    int i;
    for(i=0;i<f_num;i++)
    {
        printf("%s:\t%s\n",f_name[i],f_value[i]);
    }
    printf("--------------------------\n");
    return 0;
}

int test_db_open(char * db_name)
{
	int rc = SQLITE_OK;

	rc = sqlite3_open(db_name,&db);
	if(rc != SQLITE_OK)
	{
		printf("Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	return rc;
}

int test_db_create_table()
{
	char *errMsg;
	int rc = SQLITE_OK;
	char create[]="create table employee(staff_id integer PRIMARY KEY, name text, gender text, age integer);";

	rc = sqlite3_exec(db, create, NULL, 0, &errMsg);
	if(rc != SQLITE_OK)
	{
		printf( "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
	}
	return rc;
}
int test_db_insert()
{
	int i;
	char *errMsg;
	int rc = SQLITE_OK;
	char *insert[]={

			"insert into employee values(96,'eason','nan',33);",

			"insert into employee values(95,'wangshuanliang','nan',33);",

			"insert into employee values(97,'longruzhi','nv',24);",

			"insert into employee values(98,'lilingzhao','nan',22);",
	};

	int len = sizeof(insert)/sizeof(insert[0]);

	for(i=0;i<len;i++)
	{
		rc = sqlite3_exec(db,insert[i],NULL,NULL,&errMsg);
		if(rc != SQLITE_OK)
		{
			printf("insert record %d %s\n",i,errMsg);
			break;
		}
	}
	return rc;
}
int test_db_delete()
{
	int rc = SQLITE_OK;
	char *errMsg;
	char delete[] = "delete from employee where staff_id = 98";

	rc = sqlite3_exec(db,delete,NULL,NULL,&errMsg);
	if(rc != SQLITE_OK)
	{
		printf("delete :%s\n",errMsg);
	}

	return rc;
}
int test_db_update()
{
	int rc = SQLITE_OK;
	char *errMsg;
	char update[] = "update employee set age = 34 where staff_id = 96";

	rc = sqlite3_exec(db,update,NULL,NULL,&errMsg);
	if(rc != SQLITE_OK)
	{
		printf("update :%s\n",errMsg);
	}

	return rc;
}

int test_db_select()
{
	int rc = SQLITE_OK;
	char *errMsg;
	char select[]="select * from employee;";

	if(sqlite3_exec(db,select,select_result_callback,NULL,&errMsg)!=0)
	{
		printf("select %s\n",errMsg);
	}

	return rc;
}


void usage(char * proc_name)
{
	printf("\nUsage: %s option\n", proc_name);
	printf(	"\toption:\n"
		"\t--open            create/open db file,default name employee.db\n"
		"\t--create-table    create a table named employee\n"
		"\t--insert          insert  4 records to table employee\n"
		"\t--delete          delete  the record whose staff_id = 98\n"
		"\t--update          update  eason's age to 34\n"
		"\t--select          select all\n");
}


int main(int argc, char **argv)
{
	int rc = SQLITE_OK;
	Type testType = 0;
	int flag = 0;

	if( argc != 2 )
	{
		usage(argv[0]);
		return(1);
	}

	if(0 == rt_strcmp("--open",argv[1]))
	{
		testType = DB_OPEN;
		flag = 1;
	}
	else if(0 == rt_strcmp("--create-table",argv[1]))
	{
		testType = DB_CREATE_TABLE;
	}
	else if(0 == rt_strcmp("--insert",argv[1]))
	{
		testType = DB_INSERT;
	}
	else if(0 == rt_strcmp("--delete",argv[1]))
	{
		testType = DB_DELETE;
	}
	else if(0 == rt_strcmp("--update",argv[1]))
	{
		testType = DB_UPDATE;
	}
	else if(0 == rt_strcmp("--select",argv[1]))
	{
		testType = DB_SELECT;
	}

	if(testType >= DB_OPEN && testType <= DB_SELECT)
	{
		rc = test_db_open(DEFAULT_DB_NAME);
		if(rc == SQLITE_OK)
		{
			if(flag)printf("test open Sucess\n");
		}
		else
		{
			printf("test open Failed \n");
			return 1;
		}
	}

	switch(testType)
	{
		case DB_OPEN:
		break;
		case DB_CREATE_TABLE:
			rc = test_db_create_table();
			if(rc == SQLITE_OK)
			{
				printf("test create table Sucess\n");
			}
			else
			{
				printf("test create table Failed\n");
			}
		break;
		case DB_INSERT:
			rc = test_db_insert();
			if(rc == SQLITE_OK)
			{
				printf("test insert table Sucess\n");
			}
			else
			{
				printf("test insert table Failed\n");
			}
		break;
		case DB_DELETE:
			rc = test_db_delete();
			if(rc == SQLITE_OK)
			{
				printf("test delte record Sucess\n");
			}
			else
			{
				printf("test delte record Failed\n");
			}
		break;
		case DB_UPDATE:
			rc = test_db_update();
			if(rc == SQLITE_OK)
			{
				printf("test updat table Sucess\n");
			}
			else
			{
				printf("test updat table Failed\n");
			}
		break;
		case DB_SELECT:
			rc = test_db_select();
			if(rc == SQLITE_OK)
			{
				printf("test select  Sucess\n");
			}
			else
			{
				printf("test select  Failed\n");
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


