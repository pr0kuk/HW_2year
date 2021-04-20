#include "my_server.h"
#include "log.h"
//tcsetattr errors
static void (*send_info)(int, char*, struct sockaddr*);
int shell() //starts server's pty
{
    int ret, resfd, pid, fd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        pr_err("open fd");
        return -1;
    }
    pr_info("Bash started with fd %d", fd);
    if (grantpt(fd) < 0) {
        pr_err("grantpt %s", strerror(errno));
        return -1;
    }
    if (unlockpt(fd) < 0) {
        pr_err("unlockpt %s", strerror(errno));
        return -1;
    }
    char* path = ptsname(fd);
    if (path == NULL) {
        pr_err("ptsname");
        return -1;
    }
    resfd = open(path, O_RDWR);
    if (resfd < 0) {
        pr_err("open resfd");
        return -1;
    }
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
        if (setsid() < 0)
            pr_err("setsid");
        pr_info("bash pgid after setsid is %d", getpgid(0));
        execl("/bin/bash", "/bin/bash", NULL);
        pr_err("execl");
        exit(1);
    }
    if (pid < 0) {
        pr_err("fork");
        return -1;
    }
    return fd;
}

void read_bash(int fd, int sk, struct sockaddr* name)
{
    int ret;
    char readbuf[BUFSZ] = {0};
    struct pollfd pollfds = {fd, POLLIN};
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        if (pollfds.revents == POLLIN) {
            ret = read(fd, readbuf, BUFSZ);
            if (ret < 0) {
                pr_err("read from bash");
                return -1;
            }
            (*send_info)(sk, readbuf, name);
            if (memset(readbuf, 0, BUFSZ) == NULL) {
                pr_err("memset read_bash");
                return -1;
            }
        }
    }
}

void stop_server(int signum)
{
    pr_info("stop_server started");
    if (killpg(0, SIGKILL) == -1) {
        pr_err("killpg");
        exit(-1);
    }
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

int cmp_comm(char* child_buf, int bash_work)
{
    if (strncmp(child_buf, "quit", sizeof("quit") - 1) == 0)
        return 0;
    if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
        return 1;
    if (bash_work)
        return 2;
    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0)
        return 3;
    if (strncmp(child_buf, "shell", sizeof("shell") - 1) == 0)
        return 4;
    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0)
        return 5;
    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0)
        return 6;
    return -1;
}


int com_ls(int sk, char* child_buf, struct sockaddr* name)
{
    pr_info("ls started");
    if (memset(child_buf, 0, BUFSZ) == NULL) {
        pr_err("memset com_ls");
        return -1;
    }
    int lspipe[2], res = dup(STDOUT_FILENO), ret = 1;
    if (res == -1) {
        pr_err("res = dup(STDOUT_FILENO)");
        return -1;
    }
    if (pipe(lspipe) < 0) {
        pr_err("lspipe");
        return -1;
    }
    if (dup2(lspipe[1], 1) == -1) { 
        pr_err("dup2 lspipe[1] 1");
        return -1;
    }
    if(system("ls") < 0) {
        pr_err("execlp ls");
        return -1;
    }
    if (dup2(res, 1) < 0) {
        pr_err("dup2(res, 1)");
        return -1;
    }
    if (close(lspipe[1]) < 0) {
        pr_err("close lspipe[1]");
        return -1;
    }
    while(ret > 0) {
        ret = read(lspipe[0], child_buf, BUFSZ);
        if (ret < 0) {
            pr_err("read lspipe");
            return -1;
        }
        //pr_info("ls:\n%s", child_buf);
        (*send_info)(sk, child_buf, name);
    }
    if (close(lspipe[0]) < 0) {
        pr_err("close lspipe[0]");
        return -1;
    }
    if (close(res) < 0) {
        pr_err("close res");
        return -1;
    }
}

int com_cd(int sk, char* child_buf, struct sockaddr* name)
{
    child_buf[strlen(child_buf)] = 0;
    pr_info("chdir to %s", child_buf + sizeof("cd"));
    if (chdir(child_buf + sizeof("cd")) == -1) {
        pr_err("chdir");
        return -1;
    }
    if (memset(child_buf, 0, BUFSZ) == NULL)
        pr_err("cd memset");
    if (strcpy(child_buf, "cd completed\n") < 0) {
        pr_err("strcpy cd completed");
        return -1;
    }
    (*send_info)(sk, child_buf, name);
    return 0;
}

int execution(char* child_buf, int* bash_work, int *fd, int sk, struct sockaddr* name)
{
    enum command {quit, exit_bash, comm_to_bash, print, shell_start, cd, ls};
    switch (cmp_comm(child_buf, *bash_work)) {
        case(quit):
            stop_bash(child_buf, fd);
            if (raise(SIGKILL) != 0) {
                pr_err("raise(SIGKILL)");
                exit(-1);
            }
            break;
        case(exit_bash):
            stop_bash(child_buf, fd);
            *bash_work = 0;
            if (strcpy(child_buf, "shell terminated\n") != NULL) {
                pr_err("strcpy");
                return -1;
            }
            (*send_info)(sk, child_buf, name);
            break;
        case(comm_to_bash):
            child_buf[strlen(child_buf)] = 0, child_buf[strlen(child_buf)] = 10;
            if (write(*fd, child_buf, strlen(child_buf)) < 0) {
                pr_err("write to bash");
                return -1;
            }
            child_buf[strlen(child_buf) - 1] = 0;
            pr_info("write to bash: %s", child_buf);
            if (memset(child_buf, 0, BUFSZ) == NULL)
                pr_err("memset shell_start");
            read_bash(*fd, sk, name);
            break;
        case(print):
            pr_info("print %s", child_buf + sizeof("print "));
            (*send_info)(sk, child_buf, name);
            break;
        case(shell_start):
            *bash_work = 1;
            *fd = shell();
            read_bash(*fd, sk, name);
            if (memset(child_buf, 0, BUFSZ) == NULL)
                pr_err("memset shell_start");
            break;
        case(cd):
            com_cd(sk, child_buf, name);
            break;
        case(ls):
            com_ls(sk, child_buf, name);
            break;
        default:
            sprintf(child_buf, "unrecognized command: %s", child_buf);
            (*send_info)(sk, child_buf, name);   
    }
}

void* choose_mode(char* argv1)
{
    if (strcmp(argv1, "udp") == 0) {
        pr_info("udp loaded");
        return dlopen("./libudp.so", RTLD_LAZY);
    }
    else {
        if (strcmp(argv1, "tcp") == 0) {
            pr_info("tcp loaded");
            return dlopen("./libtcpl.so", RTLD_LAZY);
        }
        else {
            perror("argument should be tcp or udp");
            return NULL;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        perror("argc < 2");
        return -1;
    }
    int num = 0, sk, ret, mas[MAX_CLIENTS] = {0}, data_pipe[MAX_CLIENTS][2], id = 0, i = 0, ans_sk;
    log_init(NULL);
    void* sl = choose_mode(argv[1]);
    if (sl == NULL) {
        fprintf(stderr, "%s\n", dlerror());
        return -1;
    }  
    pr_info("server pgid is %d", getpgid(0));
    int id_key = shmget(SHMKEY, sizeof(unsigned int), IPC_CREAT | 0666);
    pid_t* pid = (pid_t*)shmat(id_key, NULL, 0);
    *pid = getpid();
    signal(SIGINT, stop_server);
    struct sockaddr_in name;
    pr_info("server started");
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        pr_err("answer socket");
        exit(1);
    }
    *(void **) (&send_info) = dlsym(sl, "send_info");
    void (*settings)(int *, int*, struct sockaddr_in*);
    *(void **) (&settings) = dlsym(sl, "settings");
    (*settings)(&sk, &ans_sk, &name);
    void (*server_handler)(int *, int*, int(*)[2], struct sockaddr_in*, int*, int (*)(char*, int*, int *, int, struct sockaddr*));
    *(void **) (&server_handler) = dlsym(sl, "server_handler");
    int (*executionf)(char*, int*, int *, int, struct sockaddr*) = execution;
    /*if (daemon(1,1) == -1) {
        perror("daemon");
        exit(-1);
    }*/
    while (1)
        (*server_handler)(&num, mas, data_pipe, &name, &sk, executionf);
    return 0; 
}