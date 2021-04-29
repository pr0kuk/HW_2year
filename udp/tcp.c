#include "my_server.h"
#include "log.h"
static int fork_flag = 1;

int settings(int* sk, int* ans_sk, struct sockaddr_in* name)
{
    if (log_init(NULL) < 0) {
        perror("log_init");
        return -1;
    }
    *sk = socket(AF_INET, SOCK_STREAM, 0);
    if (*sk < 0) {
        perror("socket");
        return -1;
    }
    name->sin_family = AF_INET;
    name->sin_port = htons(PORT); // htons, e.g. htons(10000)
    name->sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl, ntohl
    if (bind(*sk, (struct sockaddr*)name, sizeof(*name)) < 0) {
        perror("bind_1");
        close(*sk);
        return -1;
    }
    if (listen(*sk, 20)) {
        perror("listen");
        close(*sk);
        return -1;
    }
    return 0;
}

int send_info(int sk, char* buffer, struct sockaddr* name)
{
    crypto(buffer);
    if (write(sk, buffer, BUFSZ) < 0) {
        pr_err("send_info");
        return -1;
    }
    //pr_info("send_info: %s", buffer);
    memset(buffer, 0, BUFSZ);
    return 0;
}

int child_handle(int fork_sk, int (*execution)(char*, int, struct sockaddr*, int*), void (*off_bash)(int))
{
    char child_buf[BUFSZ] = {0};
    signal(SIGCHLD, off_bash);
    int bash_work = 0, fd;
    while (1)
    {
        if (read(fork_sk, child_buf, BUFSZ) < 0) {
            pr_err("read");
            return -1;
        }
        crypto(child_buf);
        pr_info("received: %s", child_buf)
        if (execution(child_buf, fork_sk, NULL, &fd) < 0) {
            pr_err("execution");
            return -1;
        }
    }
    return 0;
}

int server_handler(int* num, int* mas, int (*data_pipe)[2], struct sockaddr_in* name, int* sk, int (*execution)(char*, int, struct sockaddr*, int*), void (*off_bash)(int))
{
    char buffer[BUFSZ];
    pr_info("waiting for hello msg");
    int client_sk = accept(*sk, NULL, NULL);
    if (client_sk < 0) {
        pr_err("accept");
        return -1;
    }
    pr_info("accept pass");
    if (read(client_sk, buffer, BUFSZ) < 0) {
        pr_err("read");
        return -1;
    }
    pr_info("received: %s", buffer);
    pid_t child_pid = fork();
    if (child_pid == 0) {
        if (child_handle(client_sk, execution, off_bash) < 0) {
            pr_err("child_handle");
            return -1;
        }
    }
    return 0;
}