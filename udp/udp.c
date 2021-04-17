#include "my_server.h"
#include "log.h"

//\n at client doesnt work always
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


int send_info(int sk, char* buffer, struct sockaddr* name)
{
    pr_info("send info:\n%s", buffer);
    pr_info("sk is %d", sk);
    pr_info("name.sin_family=%d, name.sin_port=%d,name.sin_addr.s_addr=%d", ((struct sockaddr_in*)name)->sin_family, ((struct sockaddr_in*)name)->sin_port, ((struct sockaddr_in*)name)->sin_addr.s_addr);
    int ret = sendto(sk, buffer, BUFSZ, 0, name, sizeof(*name));
    memset(buffer, 0, BUFSZ);
    //ret = sendto(sk, buffer, BUFSZ, 0, name, sizeof(*name));
    return 0;
}

int settings(int* sk, int* ans_sk, struct sockaddr_in* name)
{

    //struct sockaddr_in name = {AF_INET, htons(PORT), htonl(INADDR_ANY)};
    name->sin_family = AF_INET;
    name->sin_port = htons(PORT); // htons, e.g. htons(10000)
    name->sin_addr.s_addr = htonl(INADDR_ANY); // htonl, ntohl
    *sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        pr_err("socket sk");
        exit(1);
    }
    if (bind(*sk, (struct sockaddr*)name, sizeof(*name)) < 0) {
        pr_err("bind sk");
        exit(1);
    }
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


void child_handle(int data_pipe_0, struct sockaddr* name) //server's suborocess working with a particular client
{
    int ret, flag = 0, fd;
    pr_info("my data_pipe_0 is %d", data_pipe_0);
    int ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        pr_err("socket ans_sk");
        exit(1);
    }
    pr_info("ans_sk is %d", ans_sk);
    pr_info("name.sin_family=%d, name.sin_port=%d,name.sin_addr.s_addr=%d", ((struct sockaddr_in*)name)->sin_family, ((struct sockaddr_in*)name)->sin_port, ((struct sockaddr_in*)name)->sin_addr.s_addr);
    pr_info("child initialized");
    while(1) {
        char child_buf[BUFSZ] = {0};
        ret = read(data_pipe_0, child_buf, BUFSZ);
        if (ret < 0)
            pr_err("read from data_pipe[my_ip][0]");
        pr_info("read in fork: %s", child_buf);
        execution(child_buf, &flag, &fd, ans_sk, name);
    }
}

int server_handler(int* num, int* mas, int (*data_pipe)[2], struct sockaddr_in* name, int* sk, int* ans_sk, struct sockaddr_in* ans_name)
{
    int ret;
    char buffer[BUFSZ + IDSZ] = {0};
    ret = recvfrom(*sk, buffer, BUFSZ + IDSZ, 0, (struct sockaddr*)name, &(int){sizeof(name)}); //getting string from client
    if (ret < 0)
        pr_err("recvfrom sk");
    pr_info("received %s", buffer);
    if (strcmp(buffer, "!hello!") == 0) //server receives broadcast
        broadcast(*ans_sk, ans_name, buffer);
    char my_ip_str[BUFSZ] ={0};
    memset(my_ip_str, 0, BUFSZ);
    if (strncmp(buffer, "!connect!", sizeof("!connect!") - 1) == 0) { // is it a first msg of client? then remember him, gave him a number and create subprocess for his commands
        if (strcpy(my_ip_str, buffer + sizeof("!connect!") -1) == NULL)
            pr_err("strcpy my_ip_str");
        int id = atoi(my_ip_str);
        *num = connect_id(id, mas);
        int int_num = *num;
        pr_info("num in so is %d", *num);
        if (*num < 0)
            pr_err("there is no room for a client");
        pr_info("connected id %d, num %d", id, *num);
        if (pipe(data_pipe[int_num]) < 0)
            pr_err("pipe");
        pid_t pid_child = fork();
        pr_info("pid_child is %d", pid_child);
        if (pid_child == 0)
            child_handle(data_pipe[*num][0], (struct sockaddr*)name);
        if (pid_child < 0)
            pr_err("fork");
    }
    else { //server works with all commands
        //pr_info("buffer before id %s", buffer);
        char buffer_without_pid[BUFSZ] = {0};
        memset(buffer_without_pid, 0, BUFSZ);
        int id = decypher(buffer, my_ip_str, buffer_without_pid);
        if (id == -1) {
            pr_err("id cannot be recognized");
            return -1;
        }
        //pr_info("id %d", id);
        *num = find(id, mas);
        if (*num < 0) {
            pr_err("find error");
            return -1;
        }
        pr_info("found id %d, num %d", id, *num);
        //buffer_without_pid[strlen(buffer_without_pid)] = 10, buffer_without_pid[strlen(buffer_without_pid)+1] = 0;
        pr_info("write to pipe[%d]: %s", *num, buffer_without_pid);
        if (write(data_pipe[*num][1], buffer_without_pid, BUFSZ) < 0) //server sends command to his particular child
            pr_err("write to data_pipe[my_ip][1]");
        if (strncmp(buffer_without_pid, "quit\n", sizeof("quit\n")) == 0) {
            close(data_pipe[*num][0]);
            close(data_pipe[*num][1]);
            mas[*num] = 0;
            pr_info("%d closed", *num);
        }
    }
    return 0;
}