#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
int flag = 1, confirmed_sender = 0;
pid_t pid;
void handler(int signum)
{
    flag = 1;
}
void send_stop(int signum)
{
    if (confirmed_sender)
        kill(pid, SIGINT);
    printf("\nsending programm was interrupted\n");
    raise(SIGKILL);
}


int main(int argc, char *argv[])
{
    int c, i;
    signal (SIGUSR1, handler);
    signal (SIGINT, send_stop);
    int mas[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    char buf[1];
    struct stat fstatbuf;
    if (argc < 3)
    {
        printf("argc < 3\n");
        return -1;
    }
    pid = atoi(argv[2]);
    printf("recpid is %d\n", pid);
    printf("senpid is %d\n", getpid());
    flag = 0;
    kill(pid, SIGCONT);
    while (flag==0);
    confirmed_sender = 1;

    int file = open(argv[1], O_RDONLY);
    if (file < 0)
    {
        perror("open");
        return -1;
    }

    fstat(file, &fstatbuf);
    if (file < 0 || ((fstatbuf.st_mode & S_IFMT) != S_IFREG))
    {
        printf("File is not regular\n");
        return 0;
    }
    clock_t start = clock();
    while (read(file, buf, 1) > 0)
    {
        c = (int)buf[0];
        i = 0;
        while (i < 8)
        {
            if (flag == 1)
            {
                flag = 0;
                if ((c&1) == 0)
                    kill(pid, SIGUSR1);
                else
                    kill(pid, SIGUSR2);
                c = c >> 1;
                i++;
            }
        }
    }
    kill(pid, SIGTERM);
    clock_t end = clock();
    printf("time of copying is %lf seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    printf("copying speed is %lf bytes/seconds\n", fstatbuf.st_size/((double)(end-start)/CLOCKS_PER_SEC));
    close(file);
    return 0;
}