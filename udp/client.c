#include "my_server.h"
int gen_id()
{
    return getpid()*rand();
}

void broadcast_client()
{
    char serv_ip[1];
    int ret, ans_sk, i, count = 0;
    struct sockaddr_in ans = {AF_INET, htons(PORT), {INADDR_BROADCAST}};
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        perror("socket ans_sk");
        exit(1);
    }
    ret = setsockopt(ans_sk, SOL_SOCKET, SO_BROADCAST, &(int){1}, sizeof(int));
    if (ret < 0) {
        perror("setsockopt");
        exit(1);
    }
    ret = sendto(ans_sk, "!hello!", sizeof("!hello!")-1, 0, (struct sockaddr*)&ans, sizeof(ans));
    if (ret < 0 || ret > sizeof("!hello!") - 1) {
        perror("broadcast");
        exit(1);
    }
    ans.sin_addr.s_addr = htonl(INADDR_ANY), ans.sin_port = 0;
    ret = bind(ans_sk, (struct sockaddr*)&ans, sizeof(ans));
    /*if (ret < 0) {
        perror("bind ans_sk"); //It is printed EINVAL, I don't know why
        exit(1);
    }*/
    getsockname(ans_sk, (struct sockaddr*)&ans, &(int){sizeof(ans)});
    printf("Broadcasting from port %d..\nWaiting for answers (%d clocks)..\n", ans.sin_port, CLOCKS_TO_WAIT);
    for (i = 0; i < CLOCKS_TO_WAIT; i++) {
        ret = recvfrom(ans_sk, serv_ip, sizeof(serv_ip), MSG_DONTWAIT, (struct sockaddr*)&ans, &(int){sizeof(ans)});
        if (ret >= 0) {
            printf("%s server found\n", inet_ntoa(ans.sin_addr));
            count++;
        }
    }
    printf("Waiting is over, %d servers found\n", count);
}

void interrupted(int signum)
{
    exit(0);
}

void cypher(int auth, int my_psd, char* buffer, char* sendbuf)
{
    int ret = sprintf(sendbuf, "%d!%s", (my_psd * auth) % KEY, buffer);
    if (ret < 0)
        perror("sprintf sendbuf+psd");
}

int sender(int id, int sk, struct sockaddr_in* name, pid_t pid)
{
    char buffer[BUFSZ] = {0}, sendbuf[BUFSZ + IDSZ] = {0};
    int ret = read(STDIN_FILENO, buffer, BUFSZ);
    if (ret < 0 || ret > BUFSZ) {
        perror("read");
        exit(1);
    }
    buffer[strlen(buffer)-1] = 0;
    ret = sprintf(sendbuf, "%d!%s", id, buffer);
    if (ret < 0) {
        perror("sprintf");
        return -1;
    }
    if (strncmp(buffer, "broadcast", sizeof("broadcast") - 1) == 0)
        broadcast_client();
    else {
        ret = sendto(sk, sendbuf, BUFSZ + IDSZ, 0, (struct sockaddr*)name, sizeof(*name));
        //printf("sent %s\n", sendbuf);
        if (ret < 0 || ret > BUFSZ + IDSZ)
            perror("sendto");
        if (strncmp(buffer, "quit", sizeof("quit") - 1) == 0) {
            kill(pid, SIGTERM);
            printf("server disconnected\n");
            exit(0);
        }
    }
}

int receiver(int sk, struct sockaddr_in* hear)
{
    char buffer_rec[BUFSZ] = {0};
    int ret;
    while(1) {
        buffer_rec[0] = 1;
        while(buffer_rec[0] != 0) {
            ret = recvfrom(sk, buffer_rec, BUFSZ, MSG_DONTWAIT, (struct sockaddr*)hear, &(int){sizeof(*hear)});
            if (ret >= 0)
                write(STDOUT_FILENO, buffer_rec, BUFSZ);
            else
                break;
            if (memset(buffer_rec, 0, BUFSZ) == NULL)
                perror("memset");
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("argc < 2\n");
        exit(1);
    }
    struct sockaddr_in name = {AF_INET, htons(PORT), 0}, hear = {AF_INET, 0, htonl(INADDR_ANY)};
    int sk, ret, i = 0;
    char pid_str[IDSZ];
    //signal(SIGINT, interrupted);
    if (inet_aton(argv[1], &name.sin_addr) == 0) {
        perror("Inputted IP-Adress");
        exit(1);
    }
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        perror("socket sk");
        exit(1);
    }
    int id = gen_id();
    printf("id is %d\n", id);
    ret = sprintf(pid_str, "%s%d", "!connect!", id);
    if (ret < 0) {
        perror("sprintf");
        exit(1);
    }
    ret = bind(sk, (struct sockaddr*)&hear, sizeof(hear));
    if (ret < 0) {
        perror("bind sk");
        exit(1);
    }
    getsockname(sk, (struct sockaddr*)&hear, &(int){sizeof(hear)});
    printf("client receiver is bind on port %d\n", hear.sin_port);
    ret = sendto(sk, pid_str, strlen(pid_str), 0, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0) {
        perror("sending first msg failed");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == 0)
        receiver(sk, &hear);
    while(1)
        sender(id, sk, &name, pid);
    return 0;
}