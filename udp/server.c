#include "my_server.h"
int main()
{
    int sk, ret;
    struct sockaddr_in name = {0};
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0)
    {
        perror("socket");
        return 1;
    }
    name.sin_family = AF_INET;
    name.sin_port = htons(23456);
    name.sin_addr.s_addr = inet_addr("127.0.0.1");
    ret = bind(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("bind");
        close(sk);
        return 1;
    }
    while (1)
    {
        int client_sk;
        char buffer[BUFSZ] = {0};
        socklen_t addrlen = sizeof(name);
        ret = recvfrom(sk, buffer, BUFSZ, 0, (struct sockaddr*)&name, &addrlen);
        if (ret < 0)
            perror("recvfrom");
        if (strncmp(buffer, "print", sizeof("print") - 1) == 0)
            printf("%s", buffer + sizeof("print"));
        if (strncmp(buffer, "exit", sizeof("exit") - 1) == 0)
            break;
        if (strncmp(buffer, "ls", sizeof("ls") - 1) == 0) {
            if (fork() == 0) {
                execlp("ls", "ls", NULL);
                perror("execlp");
            }
        }
        if (strncmp(buffer, "cd", sizeof("cd") - 1) == 0)
                if (chdir(buffer+3) == -1)
                    perror("chdir");
    }   
}