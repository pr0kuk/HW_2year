#include "my_server.h"
#include "log.h"
// tcsetattr errors
static int f_d;
int shell() //starts server's pty
{
    int ret, resfd, pid;
    int fd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
    if (fd < 0)
        pr_err("open fd");
    pr_info("Bash started with fd %d", fd);
    if (grantpt(fd) < 0)
        pr_err("grantpt %s", strerror(errno));
    if (unlockpt(fd) < 0)
        pr_err("unlockpt %s", strerror(errno));
    char* path = ptsname(fd);
    if (path == NULL)
        pr_err("ptsname");
    resfd = open(path, O_RDWR);
    if (resfd < 0)
        pr_err("open resfd");
    struct termios termios_p;
    termios_p.c_lflag = 0;
    if (tcsetattr(resfd, 0, &termios_p) < 0);
        pr_err("tcsetattr");
    pid = fork();
    if (pid == 0) {
        if (dup2(resfd, STDIN_FILENO) < 0)
            pr_err("dup2 resfd 0");
        if (dup2(resfd, STDOUT_FILENO) < 0)
            pr_err("dup2 resfd 1");
        if (dup2(resfd, STDERR_FILENO) < 0)
            pr_err("dup2 resfd 2");
        if (setsid() < 0)
            pr_err("setsid");
        execl("/bin/bash", "/bin/bash", NULL);
        pr_err("execl");
        exit(1);
    }
    if (pid < 0)
        pr_err("fork");
    f_d = fd;
    return fd;
}

void read_bash(int fd, int sk) // read from bash
{
    int ret;
    char readbuf[BUFSZ] = {0};
    struct pollfd pollfds = {fd, POLLIN};
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        ret = read(fd, readbuf, BUFSZ);
        if (ret < 0)
            pr_err("read from bash");
        pr_info("readbash sk is %d", sk);
        send_info(sk, readbuf);
        memset(readbuf, 0, BUFSZ);
    }
}

void stop_server(int signum) // exits from all bashes, kill all forks, exit
{
    pr_info("stop_server ignited");
    if (f_d < 1)
        exit(0);
    if (write(f_d, "exit\n", sizeof("exit\n")) < 0)
        pr_err("write exit to %d", f_d);
    pr_info("fd %d closed\n", f_d);
    exit(0);
}

int stop_bash(char* child_buf, int * fd)
{
    if (*fd < 3) {
        pr_info("fd %d < 3", fd);
        return -1;
    }
    if (write(*fd, "exit\n", sizeof("exit\n")) < 0) {
        pr_err("write exit to bash, fd is %d", fd);
        return -1;
    }
    pr_info("bash with fd: %d terminated", fd);
    if (close(*fd) < 0) {
        pr_err("close bash descriptor");
        return -1;
    }
    if (strcpy(child_buf, "bash terminated\n") == NULL) {
        pr_err("strcpy childbuf bash terminated");
        return -1;
    }
    *fd = 0;
    f_d = 0;
    return 0;
}



int execution(char* child_buf, int* flag, int *fd, int sk) //handles comms from client
{
    int ret;
    pr_info("execution flag = %d of child_buf: %s", *flag, child_buf);
    if (strncmp(child_buf, "quit", sizeof("quit") - 1) == 0) {
        stop_bash(child_buf, fd);
        return 0;
    }
    if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0) {
        stop_bash(child_buf, fd);
        *flag = 0;
        strcpy(child_buf, "shell terminated");
        send_info(sk, child_buf);
        return 0;
    }
    if (*flag) {
        child_buf[strlen(child_buf)] = 10, child_buf[strlen(child_buf) + 1] = 0;
        ret = write(*fd, child_buf, strlen(child_buf));
        pr_info("write to bash: %s", child_buf);
        if (ret < 0)
            pr_err("write to bash");
        read_bash(*fd, sk);
        return 0;
    }
    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0) {
        pr_info("print %s", child_buf + sizeof("print"));
        send_info(sk, child_buf);
        return 0;
    }
    if (strncmp(child_buf, "shell", sizeof("shell") - 1) == 0) {
        *flag = 1;
        pr_info("flag = 1 = %d", *flag);
        *fd = shell();
        memset(child_buf, 0, BUFSZ);
        return 0;
    }
    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0) {
        child_buf[strlen(child_buf)] = 0;
        pr_info("chdir to %s", child_buf + sizeof("cd"));
        if (chdir(child_buf + sizeof("cd")) == -1)
            pr_err("chdir");
        memset(child_buf, 0, BUFSZ);
        strcpy(child_buf, "cd completed");
        send_info(sk, child_buf);
        return 0;
    }
    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0) {
        pr_info("ls started");
        memset(child_buf, 0, BUFSZ);
        int lspipe[2], res = dup(1);
        if (pipe(lspipe) < 0)
            pr_err("lspipe");
        if (dup2(lspipe[1], 1) < 0)
            pr_err("dup2 lspipe[1] 1");
        if(system("ls") < 0)
            pr_err("execlp ls");
        if (read(lspipe[0], child_buf, BUFSZ) < 0)
            pr_err("read lspipe");
        if (dup2(res, 1) < 0)
            pr_err("dup2(res, 1)");
        if (close(lspipe[1]) < 0)
            pr_err("close lspipe[1]");
        if (close(lspipe[0]) < 0)
            pr_err("close lspipe[0]");
        if (close(res) < 0)
            pr_err("close res");
        pr_info("sent ls %s\n", child_buf);
        send_info(sk, child_buf);
        pr_info("ls: %s", child_buf);
        return 0;
    }
    char new_buf[BUFSZ] = {0};
    sprintf(new_buf, "unrecognized command: %s", child_buf);
    send_info(sk, new_buf);   
}



void child_handle(int data_pipe_0, struct sockaddr_in name) //server's suborocess working with a particular client
{
    int ret, flag = 0, fd;
    pr_info("my data_pipe_0 is %d", data_pipe_0);
    int ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        pr_err("socket ans_sk");
        exit(1);
    }    
    pr_info("child initialized");
    while(1) {
        char child_buf[BUFSZ] = {0};
        memset(child_buf, 0, BUFSZ);
        ret = read(data_pipe_0, child_buf, BUFSZ);
        if (ret < 0)
            pr_err("read from data_pipe[my_ip][0]");
        char ans_buffer[BUFSZ] = {0};
        pr_info("read in fork: %s", child_buf);
        if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
            flag = 0;
        if (flag) {
            ret = write(fd, child_buf, strlen(child_buf));
            child_buf[strlen(child_buf) - 1] = 0;
            pr_info("write to bash: %s", child_buf);
            if (ret < 0)
                pr_err("write to bash");
            read_bash(fd, 0);
        }
        else {
            execution(child_buf, &flag, &fd, 0);
            if (strncmp(child_buf, "shell", sizeof("shell") - 1)) {
                int bash_pipe[2];
                pipe(bash_pipe);
                read_bash(fd, 0);
                ret = 1;
                while(ret > 0) {
                    read(bash_pipe[0], child_buf, BUFSZ);
                    ret = sendto(ans_sk, child_buf, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
                }
                close(bash_pipe[0]);
            }
            ret = sendto(ans_sk, child_buf, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
            pr_info("sent in child_handle: %s");
        }
    }
}

int broadcast(int ans_sk, struct sockaddr_in* name, char* buffer)
{
        if (sendto(ans_sk, "", 1, 0, (struct sockaddr*)&name, sizeof(name)) != 1)
            pr_err("answer to broadcast");
        memset(buffer, 0, BUFSZ);
}


int main()
{
    int num = 0;
    log_init(NULL);
    int id_key = shmget(SHMKEY, sizeof(unsigned int), IPC_CREAT | 0666);
    pid_t* pid = (pid_t*)shmat(id_key, NULL, 0);
    *pid = getpid();
    int sk, ret, mas[MAX_CLIENTS] = {0}, data_pipe[MAX_CLIENTS][2], id = 0, i = 0, ans_sk;
    signal(SIGINT, stop_server);
    struct sockaddr_in ans = {AF_INET, htons(PORT), 0};
    struct sockaddr_in name;
    pr_info("server started");
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        pr_err("answer socket");
        exit(1);
    }
    settings(&sk, &ans_sk, &name);
    while (1)
        server_handler(&num, mas, data_pipe, &name, &sk, &ans_sk, &ans);
    return 0; 
}