#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
int i = 1, k = 0;
int mas[8];
void shandler_1(int signum)
{
    mas[k++] = 0;
}

void shandler_2(int signum)
{
    mas[k++] = 1;
}

void handler_stop(int signum)
{
    if (signum == SIGTERM)
        i = 0;
}

int main()
{

    int j = 0, d = 0;
    char buf[1];
    signal (SIGUSR1, shandler_1);
    signal (SIGUSR2, shandler_2);
    signal (SIGTERM, handler_stop);
    printf("pid is %d\n", getpid());
    int fd = open("file.copy", O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        printf("File cannot be opened");
        return 0;
    }
    while (i != 0)
    {
        if (k == 8)
        {
            d = mas[0] + mas[1] * 2 + mas[2] * 4 + mas[3] * 8 + mas[4] * 16 + mas[5] * 32 + mas[6] * 64 + mas[7] * 128;
            buf[0] = d;
            write(fd, buf, 1);
            k = 0;
        }
        if (i == 0)
            break;
        j++;
    }
    close(fd);
    return 0;
}