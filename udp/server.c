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

void read_bash(int fd, int ans_sk, struct sockaddr * name, size_t len) // read from bash
{
    int ret;
    char readbuf[BUFSZ];
    struct pollfd pollfds = {fd, POLLIN};
    for (int j = 0; j < BUFSZ; readbuf[j++] = 0);
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        for (int j = 0; j < BUFSZ; readbuf[j++] = 0);
        ret = read(fd, readbuf, BUFSZ);
        if (ret < 0)
            pr_err("read from bash");
        //pr_info("to send: %s", readbuf);
        ret = sendto(ans_sk, readbuf, BUFSZ, 0, name, len);
        if (ret < 0)
            perror("sendto ans_sk");
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



void execution(char* child_buf, int* flag, int *fd, int ans_sk, struct sockaddr* name, size_t len) //handles comms from client
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
        read_bash(*fd, ans_sk, name, len);
    }
    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0) {
            child_buf[strlen(child_buf) - 1] = 0;
            pr_info("chdir to %s", child_buf + sizeof("cd"));
            if (chdir(child_buf + sizeof("cd")) == -1)
                pr_err("chdir");
            if (strcpy(child_buf, "cd completed\n") == NULL)
                pr_err("strcpy childbuf cd completed");
            //pr_info("sent to port %d", name.sin_port);
            ret = sendto(ans_sk, child_buf, BUFSZ, 0, name, len);
            if (ret < 0)
                pr_err("sendto ans_sk");
    }
    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0) {
        int lspipe[2], res = dup(1);
        if (pipe(lspipe) < 0)
            pr_err("lspipe");
        if (dup2(lspipe[1], 1) < 0)
            pr_err("dup2 lspipe[1] 1");
        if(system("ls") < 0)
            pr_err("execlp ls");
        if (read(lspipe[0], child_buf, BUFSZ) < 0)
            pr_err("read lspipe");
        dup2(res, 1);
        close(lspipe[1]);
        close(lspipe[0]);
        close(res);
            //pr_info("sent to port %d", name.sin_port);
        ret = sendto(ans_sk, child_buf, BUFSZ, 0, name, len);
        if (ret < 0)
            pr_err("sendto ans_sk");  
    }
}



void child_handle(int sk, struct sockaddr_in name) //server's suborocess working with a particular client
{
    int ret, flag = 0, fd;
    pr_info("my sk is %d\n", sk);
    int ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        pr_err("socket ans_sk");
        exit(1);
    }    
    //pr_info("child initialized, num %d, port %d", num, name.sin_port);
    while(1) {
        char child_buf[BUFSZ] = {0};
        ret = read(sk, child_buf, BUFSZ);
        if (ret < 0)
            pr_err("read from data_pipe[my_ip][0]");
        char ans_buffer[BUFSZ] = {0};
        //pr_info("read in fork: %s", child_buf);
        if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
            flag = 0;
        if (flag) {
            ret = write(fd, child_buf, strlen(child_buf));
            child_buf[strlen(child_buf) - 1] = 0;
            pr_info("write to bash: %s", child_buf);
            if (ret < 0)
                pr_err("write to bash");
            read_bash(fd, ans_sk, (struct sockaddr*)&name, sizeof(name));
            for (int j = 0; j < BUFSZ; child_buf[j++] = 0);
        }
        else {
            execution(child_buf, &flag, &fd, ans_sk, (struct sockaddr*)&name, sizeof(name));
        }
    }
}

// ioctl
int decypher(char* buffer, char* my_ip_str, char* buffer_without_pid) //gets from client string client's id and command
{
    int i = 0;
    while (buffer[i++] != '!') {
        if (buffer[i] != '!' && buffer[i] != '-' && (buffer[i] < '0' || buffer[i] > '9')) {
                //pr_info("%c", buffer[i]); 
                return -1;
            }
    }
    if (strncpy(my_ip_str, buffer, i - 1) == NULL)
        pr_err("strncpy my_ip_str");
    if (strcpy(buffer_without_pid, buffer + i) == NULL)
        pr_err("strcpy buffer_without_pid");
    return atoi(my_ip_str);
}

int connect_id(int id, int* mas) //remembers client's ID and gives him his number
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (mas[i] == 0 || mas[i] == id) {
            mas[i] = id;
            return i;
        }
    }
    return -1;
}

int find(int id, int * mas) //find client's number by his ID
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (mas[i] == id)
            return i;
    return -1;
}


int main()
{
    log_init(NULL);
    int id_key = shmget(42, sizeof(unsigned int), IPC_CREAT | 0666);
    pid_t* pid = (pid_t*)shmat(id_key, NULL, 0);
    *pid = getpid();
    int sk, ret, mas[MAX_CLIENTS] = {0}, data_pipe[MAX_CLIENTS][2], id = 0, i = 0, ans_sk;
    struct sockaddr_in ans = {AF_INET, htons(PORT), 0};
    struct sockaddr_in name = {AF_INET, htons(PORT), htonl(INADDR_ANY)};
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    signal(SIGINT, stop_server);
    if (ans_sk < 0) {
        pr_err("answer socket");
        exit(1);
    }
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        pr_err("socket sk");
        exit(1);
    }
    ret = bind(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0) {
        pr_err("bind sk");
        exit(1);
    }
    pr_info("server started");

    
    while (1)
    {
        char my_ip_str[IDSZ] = {0};
        char buffer_without_pid[BUFSZ] = {0};
        char buffer[BUFSZ + IDSZ] = {0};
        int num, pid_child;
        ret = recvfrom(sk, buffer, BUFSZ + IDSZ, 0, (struct sockaddr*)&name, &(int){sizeof(name)}); //getting string from client
        if (ret < 0)
            pr_err("recvfrom sk");
        pr_info("received %s", buffer);
        if (strncmp(buffer, "!connect!", sizeof("!connect!") - 1) == 0) { // is it a first msg of client? then remember him, gave him a number and create subprocess for his commands
            if (strcpy(my_ip_str, buffer + sizeof("!connect!") -1) == NULL)
                pr_err("strcpy my_ip_str");
            id = atoi(my_ip_str);
            num = connect_id(id, mas);
            if (num < 0)
                write(2, "There is no room for a client or this id is already connected\n", sizeof("There is no room for a client\n"));
            pr_info("connected id %d, num %d", id, num);
            if (pipe(data_pipe[num]) < 0)
                pr_err("pipe");
            pid_child = fork();
            if (pid_child == 0) //child works with his client
                child_handle(data_pipe[num][0], name);
            if (pid_child < 0)
                pr_err("fork");
        }
        else { //server works with all commands
            if (strcmp(buffer, "!hello!") == 0) { //server receives broadcast
                ret = sendto(ans_sk, "", 1, 0, (struct sockaddr*)&name, sizeof(name));
                if (ret != 1)
                    pr_err("answer to broadcast");
                continue;
            }
            //pr_info("buffer before id %s", buffer);
            id = decypher(buffer, my_ip_str, buffer_without_pid);
            if (id == -1) {
                write(2, "id cannot be recognized\n", sizeof("id cannot be recognized\n"));
                continue;
            }
            //pr_info("id %d", id);
            num = find(id, mas);
            if (num < 0) {
                write(2, "find error\n", sizeof("find error\n"));
                continue;
            }
            pr_info("found id %d, num %d", id, num);
            buffer_without_pid[strlen(buffer_without_pid)] = 10, buffer_without_pid[strlen(buffer_without_pid)+1] = 0;
            //pr_info("write to pipe[%d]: %s\n", num, buffer_without_pid);
            if (write(data_pipe[num][1], buffer_without_pid, BUFSZ) < 0) //server sends command to his particular child
                pr_err("write to data_pipe[my_ip][1]");
            if (strncmp(buffer_without_pid, "quit\n", sizeof("quit\n")) == 0) {
                close(data_pipe[num][0]);
                close(data_pipe[num][1]);
                mas[num] = 0;
                pr_info("%d closed", num);
            }
        }
    }
    return 0; 
}