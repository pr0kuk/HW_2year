#include "my_server.h"
int mas[MAX_CLIENTS];
int fd[MAX_CLIENTS];

// check connect numbers, tcsetattr errors, output "hello!"", output print
 
void shell(int num) //starts server's pty
{
    int ret, resfd, pid;
    fd[num] = open("/dev/ptmx", O_RDWR | O_NOCTTY);
    if (fd[num] < 0)
        perror("open fd");
    if (grantpt(fd[num]) < 0)
        perror("grantpt");
    if (unlockpt(fd[num]) < 0)
        perror("unlockpt");
    char* path = ptsname(fd[num]);
    if (path == NULL)
        perror("ptsname");
    resfd = open(path, O_RDWR);
    if (resfd < 0)
        perror("open resfd");
    struct termios termios_p;
    termios_p.c_lflag = 0;
    if (tcsetattr(resfd, 0, &termios_p) < 0);
        perror("tcsetattr");
    pid = fork();
    if (pid == 0) {
        if (dup2(resfd, STDIN_FILENO) < 0)
            perror("dup2 resfd 0");
        if (dup2(resfd, STDOUT_FILENO) < 0)
            perror("dup2 resfd 1");
        if (dup2(resfd, STDERR_FILENO) < 0)
            perror("dup2 resfd 2");
        if (setsid() < 0)
            perror("setsid");
        execl("/bin/bash", "/bin/bash", NULL);
        perror("execl");
        exit(1);
    }
    if (pid < 0)
        perror("fork");
    /* Read first output of bash, usually it is a trash */
    char child_buf[BUFSZ] = {0};
    struct pollfd pollfds = {fd[num], POLLIN};
    for (int j = 0; j < BUFSZ; child_buf[j++] = 0);
    while (ret = poll(&pollfds, 1, POLL_WAIT) != 0) {
        for (int j = 0; j < BUFSZ; child_buf[j++] = 0);
        if (read(fd[num], child_buf, BUFSZ) < 0)
            perror("first read from bash");
        if (write(1, child_buf, BUFSZ) < 0)
            perror("first write from bash");
    }
    printf("\n");
}


void execution(char* child_buf, int num, int* flag) //handles comms from client
{
    int ret;
    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0)
        printf("%s\n", child_buf + sizeof("print"));
    if (strncmp(child_buf, "quit", sizeof("quit") - 1) == 0)
        exit(0);
    if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0) {
        ret = write(fd[num], "exit\n", sizeof("exit\n"));
        if (ret < 0)
            perror("write exit to bash");
        //printf("exit here\n");
        if (close(fd[num]) < 0)
            perror("close bash descriptor");
        if (strcpy(child_buf, "bash terminated\n") == NULL)
            perror("strcpy childbuf bash terminated");
    }
    if (strncmp(child_buf, "shell", sizeof("shell") - 1) == 0) {
        *flag = 1;
        shell(num);
        if (strcpy(child_buf, "shell ignited\n") == NULL)
            perror("strcpy childbuf shell inginted");
    }
    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0) {
            child_buf[strlen(child_buf) - 1] = 0;
            printf("chdir to %s\n", child_buf+sizeof("cd"));
            if (chdir(child_buf + sizeof("cd")) == -1)
                perror("chdir");
            if (strcpy(child_buf, "cd completed\n") == NULL)
                perror("strcpy childbuf cd completed");
    }
    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0) {
        int lspipe[2], res = dup(1);
        if (pipe(lspipe) < 0)
            perror("lspipe");
        if (dup2(lspipe[1], 1) < 0)
            perror("dup2 lspipe[1] 1");
        if(system("ls") < 0)
            perror("execlp ls");
        if (read(lspipe[0], child_buf, BUFSZ) < 0)
            perror("read lspipe");
        dup2(res, 1);
        close(lspipe[1]);
        close(lspipe[0]);
        close(res);        
    }
}



void child_handle(int sk, struct sockaddr_in name, int num) //server's suborocess working with a particular client
{
    int ret, flag = 0;
    int ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        perror("socket ans_sk");
        exit(1);
    }    
    //printf("child initialized, num %d, port %d\n", num, name.sin_port);
    while(1) {
        char child_buf[BUFSZ] = {0};
        ret = read(sk, child_buf, BUFSZ);
        if (ret < 0)
            perror("read from data_pipe[my_ip][0]");
        char ans_buffer[BUFSZ] = {0};
        //printf("read in fork: %s\n", child_buf);
        if (strncmp(child_buf, "exit", sizeof("exit") - 1) == 0)
            flag = 0;
        if (flag) {
            ret = write(fd[num], child_buf, strlen(child_buf)); 
            if (ret < 0)
                perror("write to bash");
            struct pollfd pollfds = {fd[num], POLLIN};
            for (int j = 0; j < BUFSZ; child_buf[j++] = 0);
            while (ret = poll(&pollfds, 1, POLL_WAIT) != 0) {
                for (int j = 0; j < BUFSZ; child_buf[j++] = 0);
                ret = read(fd[num], child_buf, BUFSZ);
                if (ret < 0)
                    perror("read from bash");
                //printf("got %s\n", child_buf);
                ret = sendto(ans_sk, child_buf, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
                if (ret < 0)
                    perror("sendto ans_sk");
            }
            for (int j = 0; j < BUFSZ; child_buf[j++] = 0);
            ret = sendto(ans_sk, child_buf, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
            if (ret < 0)
                perror("sendto ans_sk");
        }
        else {
            execution(child_buf, num, &flag);
            //printf("sent to port %d\n", name.sin_port);
            ret = sendto(ans_sk, child_buf, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
            if (ret < 0)
                perror("sendto ans_sk");
        }
    }
}

// ioctl
int decypher(char* buffer, char* my_ip_str, char* buffer_without_pid) //gets from client string client's id and command
{
    int i = 0;
    while (buffer[i++] != '!') {
        if (buffer[i] != '!' && buffer[i] != '-' && (buffer[i] < '0' || buffer[i] > '9'))
            {printf("%c\n", buffer[i]); return -1;}
    }
    if (strncpy(my_ip_str, buffer, i - 1) == NULL)
        perror("strncpy my_ip_str");
    if (strcpy(buffer_without_pid, buffer + i) == NULL)
        perror("strcpy buffer_without_pid");
    return atoi(my_ip_str);
}

int connect_id(int id) //remembers client's ID and gives him his number
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (mas[i] == 0 || mas[i] == id) {
            mas[i] = id;
            return i;
        }
    }
    return -1;
}

int find(int id) //find client's number by his ID
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (mas[i] == id)
            return i;
    return -1;
}

int main()
{
    int sk, ret, data_pipe[MAX_CLIENTS][2], id = 0, i = 0, ans_sk;
    struct sockaddr_in ans = {AF_INET, htons(PORT), 0};
    struct sockaddr_in name = {AF_INET, htons(PORT), htonl(INADDR_ANY)};
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        perror("answer socket");
        exit(1);
    }
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        perror("socket sk");
        exit(1);
    }
    ret = bind(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0) {
        perror("bind sk");
        exit(1);
    }
    printf("Server started\n");
    while (1)
    {
        char my_ip_str[IDSZ] = {0};
        char buffer_without_pid[BUFSZ] = {0};
        char buffer[BUFSZ + IDSZ] = {0};
        int num, pid;

        ret = recvfrom(sk, buffer, BUFSZ + IDSZ, 0, (struct sockaddr*)&name, &(int){sizeof(name)}); //getting string from client
        if (ret < 0)
            perror("recvfrom sk");
        //printf("received %s\n", buffer);
        if (strncmp(buffer, "!connect!", sizeof("!connect!") - 1) == 0) { // is it a first msg of client? then remember him, gave him a number and create subprocess for his commands
            if (strcpy(my_ip_str, buffer + sizeof("!connect!") -1) == NULL)
                perror("strcpy my_ip_str");
            id = atoi(my_ip_str);
            num = connect_id(id);
            if (num < 0)
                write(2, "There is no room for a client or this id is already connected\n", sizeof("There is no room for a client\n"));
            //printf("connect id %d num %d\n", id, num);
            if (pipe(data_pipe[num]) < 0)
                perror("pipe");
            pid = fork();
            if (pid == 0) //child works with his client
                child_handle(data_pipe[num][0], name, num);
            if (pid < 0)
                perror("fork");
        }
        else { //server works with all commands
            if (strcmp(buffer, "!hello!") == 0) { //server receives broadcast
                ret = sendto(ans_sk, "", 1, 0, (struct sockaddr*)&name, sizeof(name));
                if (ret != 1)
                    perror("answer to broadcast");
            }
            //printf("buffer before id %s\n", buffer);
            id = decypher(buffer, my_ip_str, buffer_without_pid);
            if (id == -1) {
                write(2, "id cannot be recognized\n", sizeof("id cannot be recognized\n"));
                continue;
            }
            //printf("id %d\n", id);
            num = find(id);
            if (num < 0) {
                write(2, "find error\n", sizeof("find error\n"));
                continue;
            }
            //printf("id %d, num %d\n", id, num);
            buffer_without_pid[strlen(buffer_without_pid)] = 10, buffer_without_pid[strlen(buffer_without_pid)+1] = 0;
            //printf("write to pipe[%d]: %s\n", num, buffer_without_pid);
            if (write(data_pipe[num][1], buffer_without_pid, BUFSZ) < 0) //server sends command to his particular child
                perror("write to data_pipe[my_ip][1]");
        }
    }
    return 0; 
}