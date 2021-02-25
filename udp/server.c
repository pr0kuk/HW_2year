#include "my_server.h"
int main()
{
    int sk, ret, data_pipe[MAX_PID][2], my_ip = 0, i = 0, ans_sk;
    char my_ip_str[MAX_PID], buffer_without_pid[BUFSZ];
    struct sockaddr_in ans = {AF_INET, htons(23456), 0};
    struct sockaddr_in name = {AF_INET, htons(23456), htonl(INADDR_ANY)};
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        perror("answer socket");
        exit(1);
    }
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        perror("socket");
        exit(1);
    }
    ret = bind(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0) {
        perror("bind");
        exit(1);
    }


    while (1)
    {
        char buffer[BUFSZ + IDSZ] = {0};
        ret = recvfrom(sk, buffer, BUFSZ + IDSZ, 0, (struct sockaddr*)&name, &(int){sizeof(name)});
        if (ret < 0)
            perror("recvfrom");
        if (strncmp(buffer, "!connect!", sizeof("!connect!") - 1) == 0) {
            if (strcpy(my_ip_str, buffer + sizeof("!connect!")-1) == NULL)
                perror("strcpy my_ip_str");
            my_ip = atoi(my_ip_str);
            if (pipe(data_pipe[my_ip]) < 0)
                perror("pipe");
            if (fork() == 0) { //child works with his client
                while(1) {
                    char child_buf[BUFSZ];
                    ret = read(data_pipe[my_ip][0], child_buf, BUFSZ);
                    if (ret < 0)
                        perror("read from data_pipe[my_ip][0]");
                    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0)
                        printf("%s\n", child_buf + sizeof("print"));
                    if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
                        exit(0);
                    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0) {
                        if (fork() == 0) {
                            execlp("ls", "ls", NULL);
                            perror("execlp");
                        }
                    }
                    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0)
                            if (chdir(child_buf + sizeof("cd")) == -1)
                                perror("chdir");
                }
            }
        }
        else { //server works with all commands
            if (strcmp(buffer, "!hello!") == 0) { //server receives broadcast
                ret = sendto(ans_sk, "", 1, 0, (struct sockaddr*)&name, sizeof(name));
                if (ret != 1)
                    perror("answer to broadcast");
            }
            i = 0;
            while (buffer[i++] != '!');
            if (strcpy(buffer_without_pid, buffer + i) == NULL)
                perror("strcpy buffer_without_pid");
            if (strncpy(my_ip_str, buffer, i-1) == NULL)
                perror("strncpy my_ip_str");
            my_ip = atoi(my_ip_str);
            ret = write(data_pipe[my_ip][1], buffer_without_pid, BUFSZ); //server sends command to his particular child
            if (ret < 0)
                perror("write to data_pipe[my_ip][1]");
        }
    }
    return 0; 
}