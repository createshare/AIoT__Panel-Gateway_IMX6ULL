#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include<time.h>
#include"artpi_cgi.h"

int main(void)
{
    char buff[50];
    int i = 0;
    int length = 0;
    memset(buff,0,sizeof(buff));
    printf("Content-Type:text/html\n\n");
    time_t timep;
    struct tm *ptime;
    time(&timep);
    ptime = localtime(&timep);

    conversion_time(ptime,buff); 
    length = strlen(buff);
    for(i = 0;i < length; i++)
    {
        if(buff[i] == '_')
        {
            buff[i] = ' ';
            break;
        }
    }
    printf("%s",buff);
    return 0;
}
