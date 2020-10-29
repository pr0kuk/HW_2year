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
#include <semaphore.h>


int c, i = 0, pid, flag = 0;
int main(int argc, char *argv[])
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    char buf[1];
    int l = 0;
    int count = 0;
    sem_t *sem = sem_open("sem", 0);
    pid = atoi(argv[2]);
    struct stat fstatbuf;
    if (argc < 3)
    {
        printf("argc < 3\n");
        return 0;
    }
    printf("start\n");
    int file = open(argv[1], O_RDONLY);
    fstat(file, &fstatbuf);
    int k = 0;
    if (file < 0 || ((fstatbuf.st_mode & S_IFMT) != S_IFREG))
    {
        printf("File is not regular\n");
        return 0;
    }



    while (read(file, buf, 1) > 0)
    {
        c = (int)buf[0], i = 0;
        printf("sending %d\n", c);
        while (i < 8)
        {
            printf("pause1\n");
            sem_wait(sem);
            if ((c&1) == 0)
                {
                    kill(pid, SIGUSR1);
                }
            else
                {
                    kill(pid, SIGUSR2);
                }
            c = c >> 1;
            i++;
            printf("pause2\n");
        }
        count++;
    }
    printf("END\n");
    sem_wait(sem);
    kill(pid, SIGTERM);
    sem_unlink("sem");
    printf("sent %d\n", count);
    clock_gettime(CLOCK_REALTIME, &end);
    printf("time of exectuing is %ld seconds and %ld nanoseconds\n", (end.tv_sec-start.tv_sec), (end.tv_nsec-start.tv_nsec));
    return 0;
}
