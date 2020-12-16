#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
int i = 1, k = 0, flag = 1;
int mas[8];
pid_t pid;
void shandler_1(int signum)
{
    mas[k++] = 0;
    flag = 1;
}

void shandler_2(int signum)
{
    mas[k++] = 1;
    flag = 1;
}

void handler_stop(int signum)
{
    i = 0;
    flag = 1;
}
void receiver_stop(int signum)
{
    kill(pid, SIGINT);
    printf("\nreceiving programm was interrupted\n");
    raise(SIGKILL);
}

int main(int argc, char* argv[])
{

    int j = 0, d = 0;
    char buf[1];
    sigset_t* sigset = (sigset_t*)malloc(sizeof(sigset_t));
    sigemptyset(sigset);
    sigaddset(sigset, SIGCONT);
    siginfo_t* siginfo = (siginfo_t*)malloc(sizeof(siginfo_t));
    sigprocmask(SIG_BLOCK, sigset, NULL);
    signal (SIGUSR1, shandler_1);
    signal (SIGUSR2, shandler_2);
    signal (SIGTERM, handler_stop);
    signal (SIGINT, receiver_stop);
    printf("recpid is %d\n", getpid());
    int signum = sigwaitinfo(sigset, siginfo);
    pid = siginfo->si_pid;
    printf("senpid is %d\n", pid);
    kill(pid, SIGUSR1);
    
    int fd = open("file.copy", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    while (i != 0)
    {
        flag = 0;
        kill(pid, SIGUSR1);
        while(flag == 0);
        if (k == 8)
        {
            d = mas[0] + mas[1] * 2 + mas[2] * 4 + mas[3] * 8 + mas[4] * 16 + mas[5] * 32 + mas[6] * 64 + mas[7] * 128;
            buf[0] = d;
            write(fd, buf, 1);
            k = 0;
        }
    }
    close(fd);
    return 0;
}