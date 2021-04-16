#include "my_server.h"
#include "log.h"
static int fork_flag = 1;

void set_flag(int signum)
{
    fork_flag = 1;
}

int settings(int* sk, int* ans_sk, struct sockaddr_in* name)
{
    *sk = socket(AF_INET, SOCK_STREAM, 0);
    if (*sk < 0)
    {
        perror("socket");
        return 1;
    }
    name->sin_family = AF_INET;
    name->sin_port = htons(PORT); // htons, e.g. htons(10000)
    name->sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl, ntohl
    signal(SIGUSR1, set_flag);
    pr_info("sk is %d", *sk);
    if (bind(*sk, (struct sockaddr*)name, sizeof(*name)) < 0)
    {
        perror("bind_1");
        close(*sk);
        return 1;
    }
    if (listen(*sk, 20))
    {
        perror("listen");
        close(*sk);
        return 1;
    }
}

int server_handler(int* num, int* mas, int (*data_pipe)[2], struct sockaddr_in* name, int* sk, int* ans_sk, struct sockaddr_in* ans_name)
{
    int client_sk = 0, ret = 0;
    char buffer[BUFSZ] = {0};
    char port_str[BUFSZ] = {0};
    memset(buffer, 0, BUFSZ);
    pr_info("waiting for hello msg");
    client_sk = accept(*sk, NULL, NULL);
    if (client_sk < 0) {
        pr_err("accept");
        exit(1);
    }
    ret = read(client_sk, buffer, BUFSZ);
    if (ret < 0 || ret > BUFSZ) {
        pr_err("read");
        exit(1);
    }
    pr_info("received: %s", buffer);
    int tcp_pipe[2];
    pipe(tcp_pipe);
    pid_t child_pid = fork();
    if (child_pid == 0) {
        struct sockaddr_in name_fork = {AF_INET, 0, inet_addr("127.0.0.1")};
        int fork_sk = socket(AF_INET, SOCK_STREAM, 0);
        if (bind(fork_sk, (struct sockaddr*)&name_fork, sizeof(name_fork)) < 0) {
            pr_err("bind");
            close(fork_sk);
            return 1;
        }
        getsockname(fork_sk, (struct sockaddr*)&name_fork, &(int){sizeof(name_fork)});
        pr_info("fork is binded on port %d", name_fork.sin_port);
        sprintf(port_str, "%d", name_fork.sin_port);
        write(tcp_pipe[1], port_str, BUFSZ);
        if (listen(fork_sk, 20)) {
            pr_err("listen");
            close(fork_sk);
            return 1;
        }
        int fork_client_sk = 0, ret = 0;
        char buffer[BUFSZ] = {0};
        fork_client_sk = accept(fork_sk, NULL, NULL);
        if (fork_client_sk < 0) {
            pr_err("accept");
            exit(1);
        }
        while (1)
        {

            ret = read(fork_client_sk, buffer, BUFSZ);
            if (ret < 0 || ret > BUFSZ) {
                pr_err("read");
                exit(1);
            }
            pr_info("to execute: %s", buffer);
            execution(buffer, NULL, NULL);
            write(fork_client_sk, buffer, BUFSZ);
            memset(buffer,0,BUFSZ);
        }
    }
    ret = read(tcp_pipe[0], port_str, BUFSZ);
    pr_info("port_str in main is %s", port_str);
    close(tcp_pipe[0]);
    close(tcp_pipe[1]);
    ret = write(client_sk, port_str, BUFSZ);
    if (ret < 0) {
        pr_err("write port");
        exit(1);
    }
}