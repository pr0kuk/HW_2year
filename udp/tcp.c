#include "my_server.h"
#include "log.h"
static int fork_flag = 1;

int send_info(int sk, char* buffer, struct sockaddr* name)
{
    crypto(buffer);
    if (write(sk, buffer, BUFSZ) < 0) {
        pr_err("send_info");
        exit(1);
    }
    //pr_info("send_info: %s", buffer);
    if (memset(buffer, 0, BUFSZ) < 0)
        pr_err("memset send_info");
}

int settings(int* sk, int* ans_sk, struct sockaddr_in* name)
{
    log_init(NULL);
    *sk = socket(AF_INET, SOCK_STREAM, 0);
    if (*sk < 0)
    {
        perror("socket");
        return 1;
    }
    name->sin_family = AF_INET;
    name->sin_port = htons(PORT); // htons, e.g. htons(10000)
    name->sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl, ntohl
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

int child_handle(int fork_sk, int (*execution)(char*, int*, int *, int, struct sockaddr*))
{
    char child_buf[BUFSZ] = {0};
    int bash_work = 0, fd = 0;
    while (1)
    {
        int ret = read(fork_sk, child_buf, BUFSZ);
        if (ret < 0 || ret > BUFSZ) {
            pr_err("read");
            exit(1);
        }
        pr_info("received: %s", child_buf)
        crypto(child_buf);
        child_buf[strlen(child_buf)-1] = 0;
        pr_info("to execute: %s", child_buf);
        execution(child_buf, &bash_work, &fd, fork_sk, NULL);
        if (memset(child_buf, 0, BUFSZ) == NULL)
            pr_err("memset child_buf");
    }
}

int server_handler(int* num, int* mas, int (*data_pipe)[2], struct sockaddr_in* name, int* sk, int (*execution)(char*, int*, int *, int, struct sockaddr*))
{
    int client_sk = 0, ret = 0, tcp_pipe[2];
    char buffer[BUFSZ], port_str[BUFSZ] = {0};
    pr_info("waiting for hello msg");
    client_sk = accept(*sk, NULL, NULL);
    if (client_sk < 0) {
        pr_err("accept");
        exit(1);
    }
    pr_info("accept pass");
    ret = read(client_sk, buffer, BUFSZ);
    if (ret < 0 || ret > BUFSZ) {
        pr_err("read");
        exit(1);
    }
    pr_info("received: %s", buffer);
    pid_t child_pid = fork();
    if (child_pid == 0)
        child_handle(client_sk, execution);
}