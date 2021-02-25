#include "my_server.h"


int main(int argc, char* argv[])
{
    struct sockaddr_in name = {0};
    int sk, ret;
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    char path[PATH_MAX];
    if (sk < 0)
    {
        perror("socket");
        exit(1);
    }
    name.sin_family = AF_INET;
    name.sin_port = htons(23456);
    name.sin_addr.s_addr = inet_addr(argv[1]);
    ret = connect(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("connect");
        close(sk);
        return 1;
    }
    pid_t pid = getpid();
    char pid_str[MAX_PID];
    char sendbuf[BUFSZ+10];
    sprintf(pid_str, "%s%d", "!connect!", pid);
    ret = sendto(sk, pid_str, strlen(pid_str), 0, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("sending first msg failed");
        exit(1);
    }
    while(1) {
        char buffer[BUFSZ] = {0};
        ret = read(1, buffer, BUFSZ);
        if (ret < 0 || ret > BUFSZ) 
        {
            perror("read");
            exit(1);
        }
        buffer[strlen(buffer)-1] = 0;
        ret = sprintf(sendbuf, "%d!%s", pid, buffer);
        if (ret < 0)
        {
            perror("sprintf");
            exit(1);
        }
        ret = sendto(sk, sendbuf, BUFSZ+10, 0, (struct sockaddr*)&name, sizeof(name));
        if (ret < 0 || ret > BUFSZ + 10)
        {
            
            perror("write");
            exit(1);
        }
        if (strncmp(buffer, "exit", sizeof("exit") - 1) == 0) {
            printf("Disconnected..");
            exit(0);
        }
    }
    return 0;
}