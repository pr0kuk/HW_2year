#include "my_server.h"
#include "log.h"
//tcsetattr errors
//static void (*send_info)(int, char*, struct sockaddr*);
static struct funcs* func;
static int bash_work, bash_pid;
enum {CMD_QUIT, CMD_SHELL, CMD_TO_BASH};


void stop_server(int signum)
{
    pr_info("stop_server");
    if (kill(bash_pid, SIGKILL) < 0)
        pr_err("kill");
    if (killpg(0, SIGKILL) < 0)
        pr_err("killpg");
    raise(SIGKILL);
}

void off_bash(int signum)
{
    pr_info("SIGCHLD");
    bash_work = 0;
}

void* choose_mode(char* argv1)
{
    if (strcmp(argv1, "udp") == 0) {
        pr_info("udp loaded");
        return dlopen("./libudp.so", RTLD_LAZY);
    }
    if (strcmp(argv1, "tcp") == 0) {
        pr_info("tcp loaded");
        return dlopen("./libtcpl.so", RTLD_LAZY);
    }
    perror("argument should be tcp or udp");
    return NULL;
}

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
    //cfmakeraw(&termios_p);
    if (tcsetattr(resfd, 0, &termios_p) < 0);
        pr_err("tcsetattr");
    pid = fork();
    if (pid == 0) {
        pr_info("bash pgid is %d", getpgid(0));
        if (dup2(resfd, STDIN_FILENO) < 0) {
            pr_err("dup2 resfd 0");
            return -1;
        }
        if (dup2(resfd, STDOUT_FILENO) < 0) {
            pr_err("dup2 resfd 1");
            return -1;
        }
        if (dup2(resfd, STDERR_FILENO) < 0) {
            pr_err("dup2 resfd 2");
            return -1;
        }
        if (setsid() < 0)
            pr_err("setsid");
        pr_info("bash pgid after setsid is %d", getpgid(0));
        bash_pid = getpid();
        execl("/bin/bash", "/bin/bash", NULL);
        pr_err("execl");
        return -1;
    }
    if (pid < 0) {
        pr_err("fork");
        return -1;
    }
    return fd;
}

int read_bash(int fd, int sk, struct sockaddr* name)
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
            if ((func->send_info(sk, readbuf, name)) < 0) {
                pr_err("send_info");
                return -1;
            }
            memset(readbuf, 0, BUFSZ);
        }
    }
    return 0;
}


int cmp_comm(char* child_buf)
{
    if (strncmp(child_buf, "quit", sizeof("quit") - 1) == 0)
        return CMD_QUIT;
    if (strncmp(child_buf, "shell", sizeof("shell") - 1) == 0)
        return CMD_SHELL;
    if (bash_work)
        return CMD_TO_BASH;
    return -1;
}



int execution(char* child_buf, int sk, struct sockaddr* name, int* fd)
{
    pr_info("CMD is %d", cmp_comm(child_buf));
    switch(cmp_comm(child_buf)) {
    case(CMD_TO_BASH):
        pr_info("to execute: %s", child_buf);
        child_buf[strlen(child_buf) + 1] = '\0';
        child_buf[strlen(child_buf)] = '\n';
        if (write(*fd, child_buf, strlen(child_buf)) < 0) {
            pr_err("write to bash");
            return -1;
        }
        memset(child_buf, 0, BUFSZ);
        if (read_bash(*fd, sk, name) < 0) {
            pr_err("read_bash");
            return -1;
        }
    break;
    case(CMD_QUIT):
        raise(SIGKILL);
    break;
    case(CMD_SHELL):
        *fd = shell();
        if (*fd < 3) {
            pr_err("shell");
            return -1;
        }
        bash_work = 1;
    break;
    default:
        pr_info("unrecognized command: %s", child_buf);
        char newbuf[BUFSZ] = {0};
        if (snprintf(newbuf, BUFSZ, "unrecognized command: %s\n", child_buf) < 0) {
            pr_err("sprintf");
            return -1;
        }
        if ((func->send_info(sk, newbuf, name)) < 0) {
            pr_err("send_info");
            return -1;
        }
    }
    memset(child_buf, 0, BUFSZ);
    return 0;
}



int main(int argc, char* argv[])
{
    if (argc < 2) {
        perror("argc < 2");
        return -1;
    }
    int num = 0, sk, ret, mas[MAX_CLIENTS] = {0}, data_pipe[MAX_CLIENTS][2], id = 0, i = 0, ans_sk;
    if (log_init(NULL) < 0) {
        perror("log_init");
        return -1;
    }
    void* sl = choose_mode(argv[1]);
    if (sl == NULL) {
        fprintf(stderr, "%s\n", dlerror());
        return -1;
    }  
    pr_info("server started, pgid is %d, pid is %d", getpgid(0), getpid());
    signal(SIGINT, stop_server);
    struct sockaddr_in name;
    printf("server started\n");
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        perror("answer socket");
        exit(1);
    }
    struct funcs func_real = {dlsym(sl, "send_info"), dlsym(sl, "settings"), dlsym(sl, "server_handler"), execution, off_bash};
    func = &func_real;
    if ((func->settings(&sk, &ans_sk, &name)) < 0) {
        perror("settings");
        return -1;
    }
    /*if (daemon(1,1) == -1) {
        perror("daemon");
        exit(-1);
    }*/
    int id_key = shmget(SHMKEY, sizeof(unsigned int), IPC_CREAT | 0666);
    pid_t* pid = (pid_t*)shmat(id_key, NULL, 0);
    *pid = getpid();
    while (1)
        if (func->server_handler(&num, mas, data_pipe, &name, &sk, func->executionf, func->off_bashf) < 0) {
            pr_err("server_handler");
        }
    return 0; 
}