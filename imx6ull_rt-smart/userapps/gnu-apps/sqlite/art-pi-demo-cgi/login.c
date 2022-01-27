#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

static sqlite3* login(char *n,char *p);
static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[]);

static int check_value = 0;
static char save_password[500];

static sqlite3* login(char *n,char *p)
{
    sqlite3 *db = NULL;
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
    db = initdb();
    if(db == NULL)
    {
        return NULL;
    }
    sqlite3_exec(db,check_user,select_result_callback,NULL,&errMsg);
    sleep(1);
    if (check_value == 0)
    {
        printf("ename");
        return NULL;
    }
    else
    {
        check_value = 0;
        value = strcmp(p,save_password);
        if (value != 0)
        {   
            printf("epass");
            return NULL;
        }
        else
        {
            rc = insert_log(db,type_log,level,operation,detail);
            if (rc != SQLITE_OK)
            {
                return NULL;
            }
            printf("success");
        }
    }
    return db;
}

static int select_result_callback(void *para, int f_num, char *f_value[], char *f_name[])
{
    int length = 0;
    length = strlen(f_value[1]);
    check_value = f_num;
    memset(save_password,0,sizeof(save_password));
    if (check_value != 0)
    {
        memcpy(save_password,f_value[1],strlen(f_value[1]));
        save_password[length] = '\0';
    }
    return f_num;
}

int main(void)
{
    sqlite3 *db = NULL;
    char *data;
    char n[20] = {0}; /* username */
    char p[50] = {0}; /* password, md5 string max 32 ch */

    printf("Content-Type:text/html\n\n");
    data = getenv("QUERY_STRING");
    sscanf(data,"n=%[^&]&p=%s", n, p);
    db = login(n, p);

    sqlite3_close(db);
    return 0;
}
