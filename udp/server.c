#include "my_server.h"
int mas[MAX_CLIENTS];
int flag[MAX_CLIENTS];
int fd[MAX_CLIENTS];
void execution(char* child_buf, int num)
{
    int ret;
    if (strncmp(child_buf, "print", sizeof("print") - 1) == 0)
        printf("%s\n", child_buf + sizeof("print"));
    if (strncmp(child_buf, "quit", sizeof("quit") - 1) == 0)
        exit(0);
    if (strncmp(child_buf, "shell", sizeof("shell") - 1) == 0) {
        flag[num] = 1;
        fd[num] = open("/dev/ptmx", O_RDWR | O_NOCTTY);
        if (fd[num] < 0) {
            perror("open fd");
        }
        ret = grantpt(fd[num]);
        if (fd[num] < 0) {
            perror("grantpt");
        }
        ret = unlockpt(fd[num]);
        if (ret < 0) {
            perror("unlockpt");
        }
        char* path = ptsname(fd[num]);
        if (path == NULL) {
            perror("ptsname");
        }
        int resfd = open(path, O_RDWR);
        struct termios termios_p;
        termios_p.c_lflag = 0;
        tcsetattr(resfd, 0, &termios_p);
        if (resfd < 0)
            perror("open path");
        if (fork() == 0) {
            ret = dup2(resfd, STDIN_FILENO);
            if (ret < 0)
                perror("dup2 resfd 0");
            ret = dup2(resfd, STDOUT_FILENO);
            if (ret < 0)
                perror("dup2 resfd 1");
            ret = dup2(resfd, STDERR_FILENO);
            if (ret < 0)
                perror("dup2 resfd 2");
            ret = setsid();
            if (ret < 0)
                perror("setsid");
            execl("/bin/bash", "/bin/bash", NULL);
            perror("execl");
            exit(1);
        }
        read(fd[num], path, BUFSZ);
        strcpy(child_buf, "shell ignited\n");
    }
    if (strncmp(child_buf, "ls", sizeof("ls") - 1) == 0) {
        int lspipe[2];
        if (pipe(lspipe) < 0)
            perror("lspipe");
        dup2(lspipe[1], 1);
        if(system("ls") < 0)
            perror("execlp");
        read(lspipe[0], child_buf, BUFSZ);
    }
    if (strncmp(child_buf, "cd", sizeof("cd") - 1) == 0) {
            if (chdir(child_buf + sizeof("cd")) == -1)
                perror("chdir");
            strcpy(child_buf, "cd completed\n");
    }
}

void child_handle(int sk, struct sockaddr_in name, int num)
{
    int ret;
    int ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        perror("socket ans_sk");
        exit(1);
    }
    while(1) {
        char child_buf[BUFSZ] = {0};
        ret = read(sk, child_buf, BUFSZ);
        if (ret < 0)
            perror("read from data_pipe[my_ip][0]");
        char ans_buffer[BUFSZ] = {0};
        if (flag[num] == 1) {
            write(fd[num], child_buf, strlen(child_buf)); 
            int ret_read = BUFSZ;
            while (ret >= (BUFSZ)) {
                char child_buf[BUFSZ] = {0};
                usleep(10000);
                ret_read = read(fd[num], child_buf, BUFSZ);
                printf("read is %d\n", ret_read);
                if (ret_read < (BUFSZ))
                    child_buf[strlen(child_buf) - 1] = 10;
                ret = sendto(ans_sk, child_buf, ret_read, 0, (struct sockaddr*)&name, sizeof(name));
                if (ret < 0)
                    perror("sendto ans_sk");
                if (ret_read < (BUFSZ))
                    break;
            }
        }
        else {
            execution(child_buf, num);
            ret = sendto(ans_sk, child_buf, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
            if (ret < 0)
                perror("sendto ans_sk");
        }
    }
}

// ioctl
int decypher(char* buffer, char* my_ip_str, char* buffer_without_pid)
{
    int i = 0;
    while (buffer[i++] != '!');
    if (strncpy(my_ip_str, buffer, i-1) == NULL)
        perror("strncpy my_ip_str");
    if (strcpy(buffer_without_pid, buffer + i) == NULL)
        perror("strcpy buffer_without_pid");
    return atoi(my_ip_str);
}

int connect_id(int id)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (mas[i] == 0) {
            mas[i] = id;
            return i;
        }
    }
    return -1;
}

int find(int id)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (mas[i] == id)
            return i;
    }
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

    while (1)
    {
        char my_ip_str[IDSZ] = {0};
        char buffer_without_pid[BUFSZ] = {0};
        char buffer[BUFSZ + IDSZ] = {0};
        int num;
        ret = recvfrom(sk, buffer, BUFSZ + IDSZ, 0, (struct sockaddr*)&name, &(int){sizeof(name)});
        if (ret < 0)
            perror("recvfrom sk");
        printf("received %s\n", buffer);
        if (strncmp(buffer, "!connect!", sizeof("!connect!") - 1) == 0) {
            if (strcpy(my_ip_str, buffer + sizeof("!connect!")-1) == NULL)
                perror("strcpy my_ip_str");
            id = atoi(my_ip_str);
            num = connect_id(id);
            if (num < 0) {
                write(2, "There is no room for a client\n", sizeof("There is no room for a client\n"));
            }
            printf("id num %d %d\n", id, num);
            if (pipe(data_pipe[num]) < 0)
                perror("pipe");
            if (fork() == 0) //child works with his client
                child_handle(data_pipe[num][0], name, num);
        }
        else { //server works with all commands
            if (strcmp(buffer, "!hello!") == 0) { //server receives broadcast
                ret = sendto(ans_sk, "", 1, 0, (struct sockaddr*)&name, sizeof(name));
                if (ret != 1)
                    perror("answer to broadcast");
            }
            printf("buffer before id %s\n", buffer);
            id = decypher(buffer, my_ip_str, buffer_without_pid);
            printf("id %d\n", id);
            num = find(id);
            printf("id num %d %d\n", id, num);
            if (num < 0) {
                write(2, "find error\n", sizeof("find error\n"));
            }
            if (strncmp(buffer_without_pid, "shell", sizeof("shell")-1) == 0)
                flag[num] = 1;
            if (flag[num] == 1)
                buffer_without_pid[strlen(buffer_without_pid)] = 10, buffer_without_pid[strlen(buffer_without_pid)+1] = 0;
            ret = write(data_pipe[num][1], buffer_without_pid, BUFSZ); //server sends command to his particular child
            if (ret < 0)
                perror("write to data_pipe[my_ip][1]");
        }
    }
    return 0; 
}