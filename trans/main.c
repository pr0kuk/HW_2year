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
    int ret = sizeof(void*);
    union sigval sv;
    signal (SIGUSR1, handler);
    signal (SIGINT, send_stop);
    char buf[sizeof(void*)];
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
    kill(pid, SIGCHLD);
    while (flag == 0);
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
    while (ret == sizeof(void*))
    {
        ret = read(file, buf, sizeof(void*));
        flag = 0;
        if (ret < sizeof(void*))
        {
            kill(pid, SIGTERM);
            buf[ret] = 0;
        }
        memcpy(&(sv.sival_ptr), buf, sizeof(void*));
        sigqueue(pid, SIGUSR1, sv);
        while (flag == 0);
    }
    kill(pid, SIGTERM);
    clock_t end = clock();

    
    printf("time of copying is %lf seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    printf("copying speed is %lf bytes/seconds\n", fstatbuf.st_size/((double)(end-start)/CLOCKS_PER_SEC));
    close(file);
    return 0;
}