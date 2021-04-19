#include "my_server.h"
#include "log.h"
// tcsetattr errors
static void* sl;
static void (*send_info)(int, char*, struct sockaddr*);
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
        pr_info("bash pgid is %d", getpgid(0));
        if (dup2(resfd, STDIN_FILENO) < 0)
            pr_err("dup2 resfd 0");
        if (dup2(resfd, STDOUT_FILENO) < 0)
            pr_err("dup2 resfd 1");
        if (dup2(resfd, STDERR_FILENO) < 0)
            pr_err("dup2 resfd 2");
        //if (setsid() < 0)
        //    pr_err("setsid");
        pr_info("bash pgid after setsid is %d", getpgid(0));
        execl("/bin/bash", "/bin/bash", NULL);
        pr_err("execl");
        exit(1);
    }
    if (pid < 0)
        pr_err("fork");
    return fd;
}

void read_bash(int fd, int sk, struct sockaddr* name)
{
    int ret;
    char readbuf[BUFSZ] = {0};
    struct pollfd pollfds = {fd, POLLIN};
    *(void **) (&send_info) = dlsym(sl, "send_info");
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        ret = read(fd, readbuf, BUFSZ);
        if (ret < 0)
            pr_err("read from bash");
        //pr_info("readbash sk is %d", sk);
        //send_info(sk, readbuf, name);
        (*send_info)(sk, readbuf, name);
        memset(readbuf, 0, BUFSZ);
    }
}

void stop_server(int signum)
{
    pr_info("stop_server started");
    killpg(0, SIGKILL);
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
    *fd = 0;
    return 0;
}



int execution(char* child_buf, int* flag, int *fd, int sk, struct sockaddr* name)
{
    int ret;
    *(void **) (&send_info) = dlsym(sl, "send_info");
    pr_info("execution flag = %d of child_buf: %s", *flag, child_buf);
    if (strncmp(child_buf, "quit", sizeof("quit") - 1) == 0) {
        stop_bash(child_buf, fd);
        raise(SIGKILL);
    }
    if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0) {
        stop_bash(child_buf, fd);
        *flag = 0;
        strcpy(child_buf, "shell terminated\n");
        //send_info(sk, child_buf, name);
        (*send_info)(sk, child_buf, name);
        return 0;
    }
    if (*flag) {
        child_buf[strlen(child_buf)] = 10, child_buf[strlen(child_buf) + 1] = 0;
        ret = write(*fd, child_buf, strlen(child_buf));
        child_buf[strlen(child_buf) - 1] = 0;
        pr_info("write to bash: %s", child_buf);
        if (ret < 0)
            pr_err("write to bash");
        memset(child_buf, 0, BUFSZ);
        read_bash(*fd, sk, name);
        return 0;
    }
    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0) {
        pr_info("print %s", child_buf + sizeof("print"));
        //send_info(sk, child_buf, name);
        (*send_info)(sk, child_buf, name);
        return 0;
    }
    if (strncmp(child_buf, "shell", sizeof("shell") - 1) == 0) {
        *flag = 1;
        pr_info("flag = 1 = %d", *flag);
        *fd = shell();
        read_bash(*fd, sk, name);
        memset(child_buf, 0, BUFSZ);
        return 0;
    }
    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0) {
        child_buf[strlen(child_buf)] = 0;
        pr_info("chdir to %s", child_buf + sizeof("cd"));
        if (chdir(child_buf + sizeof("cd")) == -1)
            pr_err("chdir");
        memset(child_buf, 0, BUFSZ);
        strcpy(child_buf, "cd completed\n");
        (*send_info)(sk, child_buf, name);
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
        if (dup2(res, 1) < 0)
            pr_err("dup2(res, 1)");
        if (close(lspipe[1]) < 0)
            pr_err("close lspipe[1]");
        ret = 1;
        while(ret > 0) {
            ret = read(lspipe[0], child_buf, BUFSZ);
            if (ret < 0)
                pr_err("read lspipe");
            //pr_info("ls:\n%s", child_buf);
            (*send_info)(sk, child_buf, name);
        }
        if (close(lspipe[0]) < 0)
            pr_err("close lspipe[0]");
        if (close(res) < 0)
            pr_err("close res");
        return 0;
    }
    char new_buf[BUFSZ] = {0};
    sprintf(new_buf, "unrecognized command: %s", child_buf);
    (*send_info)(sk, new_buf, name);   
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        perror("argc < 2");
        return -1;
    }
    int num = 0;
    log_init(NULL);
    if (strcmp(argv[1], "udp") == 0) {
        sl = dlopen("./libudp.so", RTLD_LAZY);
        if (sl == NULL) {
            fprintf(stderr, "%s\n", dlerror());
            return -1;
        }  
        pr_info("udp loaded");
    }
    else {
        if (strcmp(argv[1], "tcp") == 0) {
            sl = dlopen("./libtcpl.so", RTLD_LAZY);
            if (sl == NULL) {
                fprintf(stderr, "%s\n", dlerror());
                return -1;
            }
            pr_info("tcp loaded");
        }
        else {
            perror("argument should be tcp or udp");
            return -1;
        }
    }
    if (daemon(1,1) == -1)
    {
        perror("daemon");
        exit(-1);
    }
    pr_info("server pgid is %d", getpgid(0));
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
    void (*settings)(int *, int*, struct sockaddr_in*);
    *(void **) (&settings) = dlsym(sl, "settings");
    //settings(&sk, &ans_sk, &name);
    (*settings)(&sk, &ans_sk, &name);
    void (*server_handler)(int *, int*, int(*)[2], struct sockaddr_in*, int*, int (*)(char*, int*, int *, int, struct sockaddr*));
    *(void **) (&server_handler) = dlsym(sl, "server_handler");
    int (*executionf)(char*, int*, int *, int, struct sockaddr*) = execution;
    while (1) {
        //server_handler(&num, mas, data_pipe, &name, &sk, &ans_sk, &ans);
        (*server_handler)(&num, mas, data_pipe, &name, &sk, executionf);
    }
    return 0; 
}