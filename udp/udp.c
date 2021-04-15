#include "my_server.h"
#include "log.h"
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

int server_handler(char* buffer, int* num, int* mas, int (*data_pipe)[2], struct sockaddr_in* name)
{
    char my_ip_str[BUFSZ];
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
        if (pid_child == 0) //child works with his client
            child_handle(data_pipe[*num][0], *name);
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
        buffer_without_pid[strlen(buffer_without_pid)] = 10, buffer_without_pid[strlen(buffer_without_pid)+1] = 0;
        //pr_info("write to pipe[%d]: %s\n", num, buffer_without_pid);
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