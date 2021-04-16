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

void read_bash(int fd, int* bash_pipe) // read from bash
{
    int ret;
    char readbuf[BUFSZ];
    struct pollfd pollfds = {fd, POLLIN};
    //for (int j = 0; j < BUFSZ; readbuf[j++] = 0); //memset
    memset(readbuf, 0, BUFSZ);
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        //for (int j = 0; j < BUFSZ; readbuf[j++] = 0);
        memset(readbuf, 0, BUFSZ);
        ret = read(fd, readbuf, BUFSZ);
        if (ret < 0)
            pr_err("read from bash");
        pr_info("to send: %s", readbuf);
        write(bash_pipe[1], readbuf, BUFSZ);
        //ret = sendto(ans_sk, readbuf, BUFSZ, 0, name, len);
        //if (ret < 0)
           // perror("sendto ans_sk");
    }
    close(bash_pipe[1]);
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



void execution(char* child_buf, int* flag, int *fd) //handles comms from client
{
    int ret;
    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0)
        pr_info("%s", child_buf + sizeof("print"));
    if (strncmp(child_buf, "quit", sizeof("quit") - 1) == 0) {
        stop_bash(child_buf, fd);
        exit(0);
    }
    if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
        stop_bash(child_buf, fd);
    if (strncmp(child_buf, "shell", sizeof("shell") - 1) == 0) {
        *flag = 1;
        *fd = shell();
        memset(child_buf, 0, BUFSZ);
        strncpy(child_buf, "shell", sizeof("shell") - 1);
    }
    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0) {
            child_buf[strlen(child_buf) - 1] = 0;
            pr_info("chdir to %s", child_buf + sizeof("cd"));
            if (chdir(child_buf + sizeof("cd")) == -1)
                pr_err("chdir");
            memset(child_buf, 0, BUFSZ);
            if (strcpy(child_buf, "cd completed\n") == NULL)
                pr_err("strcpy childbuf cd completed");
            //pr_info("sent to port %d", name.sin_port);
            //ret = sendto(ans_sk, child_buf, BUFSZ, 0, name, len);
            pr_info("sent %s\n", child_buf);
            if (ret < 0)
                pr_err("sendto ans_sk");
    }
    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0) {
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
            //pr_info("sent to port %d", name.sin_port);
        //ret = sendto(ans_sk, child_buf, BUFSZ, 0, name, len);
        pr_info("sent %s\n", child_buf);
       // if (ret < 0)
          //  pr_err("sendto ans_sk");  
    }
}



void child_handle(int sk, struct sockaddr_in name) //server's suborocess working with a particular client
{
    int ret, flag = 0, fd;
    pr_info("my sk is %d", sk);
    int ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        pr_err("socket ans_sk");
        exit(1);
    }    
    pr_info("child initialized");
    while(1) {
        char child_buf[BUFSZ] = {0};
        memset(child_buf, 0, BUFSZ);
        ret = read(sk, child_buf, BUFSZ);
        if (ret < 0)
            pr_err("read from data_pipe[my_ip][0]");
        char ans_buffer[BUFSZ] = {0};
        memset(ans_buffer, 0, BUFSZ);
        pr_info("read in fork: %s", child_buf);
        if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
            flag = 0;
        if (flag) {
            ret = write(fd, child_buf, strlen(child_buf));
            child_buf[strlen(child_buf) - 1] = 0;
            pr_info("write to bash: %s", child_buf);
            if (ret < 0)
                pr_err("write to bash");
            int bash_pipe[2];
            pipe(bash_pipe);
            read_bash(fd, bash_pipe);
            ret = 1;
            while(ret > 0) {
                read(bash_pipe[0], child_buf, BUFSZ);
                ret = sendto(ans_sk, child_buf, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
            }
            close(bash_pipe[0]);
            //for (int j = 0; j < BUFSZ; child_buf[j++] = 0);
            memset(child_buf, 0, BUFSZ);
        }
        else {
            execution(child_buf, &flag, &fd);
            if (strncmp(child_buf, "shell", sizeof("shell") - 1)) {
                int bash_pipe[2];
                pipe(bash_pipe);
                read_bash(fd, bash_pipe);
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

