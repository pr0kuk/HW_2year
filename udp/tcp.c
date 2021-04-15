#include "my_server.h"
#include "log.h"
static int fork_flag = 0;

void set_flag(int signum)
{
    fork_flag = 1;
}


int server_handler(char* buffer, int* num, int* mas, int (*data_pipe)[2], struct sockaddr_in* name)
{
    signal(SIGUSR1, set_flag);
    int sk = socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0)
    {
        perror("socket");
        return 1;
    }
    pid_t pid_parent = getpid();
    while(1) {
        if (fork_flag) {
            fork_flag = 0;
            pid_t pid_child = fork();
            if (pid_child == 0)
            {
                if (bind(sk, (struct sockaddr*)&name, sizeof(name)) < 0)
                {
                    perror("bind");
                    close(sk);
                    return 1;
                }
                if (listen(sk, 20))
                {
                    perror("listen");
                    close(sk);
                    return 1;
                }
                kill(pid_parent, SIGUSR1);
                while (1)
                {
                    int client_sk = 0, ret = 0;
                    char buffer[BUFSZ] = {0};
                    memset(buffer, 0, BUFSZ);
                    client_sk = accept(sk, NULL, NULL);
                    if (client_sk < 0)
                    {
                        perror("accept");
                        exit(1);
                    }
                    ret = read(client_sk, buffer, BUFSZ);
                    if (ret < 0 || ret > BUFSZ)
                    {
                        perror("read");
                        exit(1);
                    }
                    close(client_sk);
                    buffer[strlen(buffer)] = 10, buffer[strlen(buffer)+1] = 0;
                    //pr_info("write to pipe[%d]: %s\n", num, buffer_without_pid);
                    if (write(data_pipe[*num][1], buffer, BUFSZ) < 0) //server sends command to his particular child
                        pr_err("write to data_pipe[my_ip][1]");
                }
            }
        }
    }
}