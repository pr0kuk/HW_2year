#include "my_server.h"
#include "log.h"



int settings(int* sk, int* ans_sk, struct sockaddr_in* name)
{
    if (log_init(NULL) < 0) {
        perror("log_init");
        return -1;
    }
    name->sin_family = AF_INET;
    name->sin_port = htons(PORT); // htons, e.g. htons(10000)
    name->sin_addr.s_addr = htonl(INADDR_ANY); // htonl, ntohl
    *sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        pr_err("socket sk");
        return -1;
    }
    if (bind(*sk, (struct sockaddr*)name, sizeof(*name)) < 0) {
        pr_err("bind sk");
        return -1;
    }
}

int broadcast(int ans_sk, struct sockaddr_in name)
{
    pr_info("broadcast name: %d %d %d", name.sin_family, name.sin_port, name.sin_addr.s_addr);
    if (sendto(ans_sk, "", 1, 0, (struct sockaddr*)&name, sizeof(name)) != 1) {
        pr_err("answer to broadcast");
        return -1;
    }
    return 0;
}

int decypher(char* buffer, char* my_ip_str, char* buffer_without_pid) //gets from client string client's id and command
{
    if (strncpy(my_ip_str, buffer, IDSZ - 1) == NULL) {
        pr_err("strncpy my_ip_str");
        return -1;
    }
    if (strcpy(buffer_without_pid, buffer + IDSZ - 1) == NULL) {
        pr_err("strcpy buffer_without_pid");
        return -1;
    }
    return atoi(my_ip_str);
}

int connect_id(int id, int * mas) //remembers client's ID and gives him his number
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

int send_info(int sk, char* buffer, struct sockaddr* name)
{
    crypto(buffer);
    //pr_info("send info:\n%s", buffer);
    //pr_info("sk is %d", sk);
    //pr_info("name.sin_family=%d, name.sin_port=%d,name.sin_addr.s_addr=%d", ((struct sockaddr_in*)name)->sin_family, ((struct sockaddr_in*)name)->sin_port, ((struct sockaddr_in*)name)->sin_addr.s_addr);
    int ret = sendto(sk, buffer, BUFSZ, 0, name, sizeof(*name)); 
    if (ret < 0 || ret > BUFSZ) {
        pr_err("sendto")
        return -1;
    }
    memset(buffer, 0, BUFSZ);
    return 0;
}


int child_handle(int data_pipe_0, struct sockaddr* name, int(*execution)(char*, int, struct sockaddr*, int*), void(*off_bash)(int)) //server's suborocess working with a particular client
{
    signal(SIGCHLD, off_bash);
    pr_info("IM HERE");
    int fd, ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        pr_err("socket ans_sk");
        return -1;
    }
    while(1) {
        char child_buf[BUFSZ] = {0};
        if (read(data_pipe_0, child_buf, BUFSZ) < 0) {
            pr_err("read from data_pipe[my_ip][0]");
            return -1;
        }
        pr_info("read from pipe: %s", child_buf);
        if (execution(child_buf, ans_sk, name, &fd) < 0) {
            pr_err("execution");
            return -1;
        }
    }
    return 0;
}


int com_connect(int* num, int* mas, char* buffer, int (*data_pipe)[2], struct sockaddr_in* name, int (*execution)(char*, int, struct sockaddr*, int*), void (*off_bash)(int))
{
    char my_ip_str[BUFSZ] = {0};
    if (strncpy(my_ip_str, buffer + sizeof("!connect!") - 1, IDSZ) == NULL) {
        pr_err("strcpy my_ip_str");
        return -1;
    }
    pr_info("my_ip_str, com_connect: %s", my_ip_str);
    int id = atoi(my_ip_str);
    *num = connect_id(id, mas);
    if (*num < 0) {
        pr_err("connect_id");
        return -1;
    }
    int int_num = *num;
    pr_info("num in server_handler is %d", *num);
    if (*num < 0) {
        pr_err("there is no room for a client");
        return -1;
    }
    pr_info("connected id %d, num %d", id, *num);
    if (pipe(data_pipe[int_num]) < 0) {
        pr_err("pipe");
        return -1;
    }
    pid_t pid_child = fork();
    if (pid_child == 0) {
        if (child_handle(data_pipe[*num][0], (struct sockaddr*)name, execution, off_bash) < 0) {
            pr_err("child_handle");
            return -1;
        }
    }
    if (pid_child < 0) {
        pr_err("fork");
        return -1;
    }
    return 0;
}

int master_handler(int (*data_pipe)[2], int* num, char* buffer, int* mas)
{
    crypto(buffer);
    char buffer_without_pid[BUFSZ] = {0}, my_ip_str[BUFSZ] = {0};
    int id = decypher(buffer, my_ip_str, buffer_without_pid);
    if (id == -1) {
        pr_err("id cannot be recognized");
        return -1;
    }
    *num = find(id, mas);
    if (*num < 0) {
        pr_err("find error");
        return -1;
    }
    pr_info("found id %d, num %d", id, *num);
    pr_info("write to pipe[%d]: %s", *num, buffer_without_pid);
    if (write(data_pipe[*num][1], buffer_without_pid, BUFSZ) < 0) {
        pr_err("write to data_pipe[my_ip][1]");
        return -1;
    }
    if (strncmp(buffer_without_pid, "quit\n", sizeof("quit\n")) == 0) {
        if (close(data_pipe[*num][0]) == -1) {
            pr_err("close data_pipe[*num][0]");
            return -1;
        }
        if (close(data_pipe[*num][1]) == -1) {
            pr_err("close data_pipe[*num][1]");
            return -1;
        }
        mas[*num] = 0;
        pr_info("%d disconnected", *num);
    }
    return 0;
}

int server_handler(int* num, int* mas, int (*data_pipe)[2], struct sockaddr_in* name, int* sk, int (*execution)(char*, int, struct sockaddr*, int*), void (*off_bash)(int))
{
    char buffer[BUFSZ + IDSZ] = {0};
    if (recvfrom(*sk, buffer, BUFSZ + IDSZ, 0, (struct sockaddr*)name, &(int){sizeof(name)}) < 0) {
        pr_err("recvfrom sk");
        return -1;
    }
    if (strcmp(buffer, "!hello!") == 0) {
        if (broadcast(*sk, *name) < 0) {
            pr_err("broadcast");
            return -1;
        }
        return 0;
    }
    if (strncmp(buffer, "!connect!", sizeof("!connect!") - 1) == 0) {
        if (com_connect(num, mas, buffer, data_pipe, name, execution, off_bash) < 0) {
            pr_err("com_connect");
            return -1;
        }
    }
    else {
        if (master_handler(data_pipe, num, buffer, mas) < 0) {
            pr_err("master_handler");
            return -1;
        }
    }
    return 0;
}