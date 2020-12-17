#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
int i = 1, flag = 1, ignition = 0;
void* mem;
pid_t pid;
void shandler_1(int sig, siginfo_t *si, void *ucontext)
{
    memcpy(mem, &(si->si_value.sival_ptr), sizeof(void*));
    flag = 1;
}

void handler_stop(int signum)
{
    i = 0;
}

void receiver_stop(int signum)
{
    kill(pid, SIGINT);
    printf("\nreceiving programm was interrupted\n");
    raise(SIGKILL);
}
void killrest(int signum, siginfo_t *siginfo, void* context)
{
    if (pid != 0 && siginfo->si_pid != pid)
    {
        kill(siginfo->si_pid, SIGKILL);
        printf("sender %d was killed\n", siginfo->si_pid);
    }
    else
    {
        ignition = 1;
        pid = siginfo->si_pid;
        printf("senpid is %d\n", siginfo->si_pid);
    }
}
int main(int argc, char* argv[])
{

    mem = malloc(sizeof(void*));
    char temp[1];
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = shandler_1;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);
    signal (SIGTERM, handler_stop);
    signal (SIGINT, receiver_stop);
    printf("recpid is %d\n", getpid());
    struct sigaction killing;
    killing.sa_flags = SA_SIGINFO;
    killing.sa_sigaction = &killrest;
    sigaction(SIGCHLD, &killing, NULL);
    int fd = open("file.copy", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }
    while (ignition == 0);


    while (1)
    {
        flag = 0;
        kill(pid, SIGUSR1);
        while(flag == 0);
        if (i == 0)
            break;
        write(fd, (char*)mem, sizeof(void*));
    }
    
    write(fd, (char*)mem, strlen((char*)mem));
    kill(pid, SIGUSR1);
    close(fd);
    return 0;
}