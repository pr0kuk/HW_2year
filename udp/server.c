#include "my_server.h"
int main()
{
    int sk, ret, data_pipe[MAX_PID][2];
    char my_ip_str[MAX_PID];
    char buffer_without_pid[BUFSZ];
    int my_ip;
    int i = 0;
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
    char buffer[BUFSZ] = {0};
    socklen_t addrlen = sizeof(name);
    ret = bind(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("bind");
        close(sk);
        return 1;
    }
    while (1)
    {
        ret = recvfrom(sk, buffer, BUFSZ, 0, (struct sockaddr*)&name, &addrlen);
        if (ret < 0)
            perror("recvfrom");
        if (strncmp(buffer, "!connect!", sizeof("!connect!") - 1) == 0) {
            if (strcpy(my_ip_str, buffer+9) == NULL) {
                perror("strcpy");
                exit(1);
            }
            my_ip = atoi(my_ip_str);
            ret = pipe(data_pipe[my_ip]);
            if (ret < 0) {
                perror("pipe");
                exit(1);
            }
            if (fork() == 0) {
                while(1) {
                    char child_buf[BUFSZ];
                    ret = read(data_pipe[my_ip][0], child_buf, BUFSZ);
                    if (ret < 0) {
                        perror("read");
                        exit(1);
                    }
                    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0)
                        printf("%s", child_buf + sizeof("print"));
                    if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
                        exit(0);
                    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0) {
                        if (fork() == 0) {
                            execlp("ls", "ls", NULL);
                            perror("execlp");
                        }
                    }
                    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0)
                            if (chdir(child_buf+3) == -1)
                                perror("chdir");
                }
            }
        }
        else {
            //if (strcmp(buffer, "!hello!") == 0)
            i = 0;
            while (buffer[i++] != '!');
            if (strcpy(buffer_without_pid, buffer + i) == NULL) {
                perror("strcpy");
                exit(1);
            }
            if (strncpy(my_ip_str, buffer, i-1) == NULL) {
                perror("strncpy");
                exit(1);
            }
            my_ip = atoi(my_ip_str);
            ret = write(data_pipe[my_ip][1], buffer_without_pid, BUFSZ);
            if (ret < 0) {
                perror("write");
                exit(1);
            }
        }
    } 
}