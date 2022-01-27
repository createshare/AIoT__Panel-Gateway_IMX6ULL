#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char read_buffer[100];
char write_buffer[] = "hello rtthread\n";
size_t read_len = 100;
int main(int argc, char *argv[])
{
    int fd = -1;
    int ret = 0;
    int run_count = 0;
    fd_set readset;
    printf("start uart test\n");
    fd = open("/dev/uart3",O_RDWR); 
    if(fd < 0)
    {
        printf("open /dev/uart3 failed\n");
        return -1;
    }
    else
    {
        printf("open /dev/uart3 success\n");
    }

    write(fd,write_buffer,(strlen(write_buffer)+1));

    while(1)
    {
        FD_ZERO(&readset);
        FD_SET(fd, &readset);
        if (select(fd + 1, &readset, NULL, NULL, NULL) == 0)
        {
            continue;
        }
        ret = read(fd,read_buffer,read_len);        
        if(ret >0)
        {
            printf("\nret %d read_buffer:\n",ret);
            for(int i=0;i<ret;i++)
            {
                printf("0x%02x,",read_buffer[i]);
            }
            printf("\n");

        }
        else
        {
            printf("read error %d\n",ret);
            break;
        }  

        write(fd,write_buffer,(strlen(write_buffer)+1));

        if(run_count > 10)
        {
            printf("close fd\n");
            close(fd);        
            return 0;
        }
        run_count++;
    }

    close(fd);
    return 0;
}