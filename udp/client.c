#include "my_server.h"
static int sk;
struct sockaddr_in* name;
char id[IDSZ];

int gen_id()
{
    char temp[IDSZ];
    memset(temp, '0', IDSZ);
    int rid = getpid()*rand();
    if (snprintf(id, IDSZ, "%d%s", rid, temp) < 0) {
        perror("snprintf");
        return -1;
    }
    printf("id is %s\n", id);
    return 0;
}


int broadcast_client()
{
    char serv_ip[1];
    int ret, ans_sk, i, count = 0;
    struct sockaddr_in ans = {AF_INET, 0, {htonl(INADDR_ANY)}};
    ans_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (ans_sk < 0) {
        perror("socket ans_sk");
        return -1;
    }
    if (bind(ans_sk, (struct sockaddr*)&ans, sizeof(ans)) < 0) {
        perror("bind ans_sk");
        return -1;;
    }
    ans.sin_addr.s_addr = htonl(INADDR_BROADCAST), ans.sin_port = htons(PORT);
    if (setsockopt(ans_sk, SOL_SOCKET, SO_BROADCAST, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt");
        return -1;
    }
    ret = sendto(ans_sk, "!hello!", BUFSZ, 0, (struct sockaddr*)&ans, sizeof(ans));
    if (ret < 0 || ret > BUFSZ) {
        perror("broadcast");
        return -1;
    }
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
    return 0;
}

void interrupted(int signum)
{
    printf("interrupted\n");
    char buffer[BUFSZ] = {0};
    if (snprintf(buffer, BUFSZ, "%s%s", id, "quit") < 0) {
        perror("snprintf");
    }
    if (sendto(sk, buffer, BUFSZ, 0, (struct sockaddr*)name, sizeof(*name)) < 0) {
        perror("sigint sendto");
    }
    raise(SIGKILL);
}

int cypher(int auth, int my_psd, char* buffer, char* sendbuf)
{
    if (sprintf(sendbuf, "%d!%s", (my_psd * auth) % KEY, buffer) < 0)
        perror("sprintf sendbuf+psd");
        return -1;
}

int read_receiver(int* data_pipe) {
    struct pollfd pollfds = {data_pipe[0], POLLIN};
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        char buffer[BUFSZ] = {0};
        if (pollfds.revents == POLLIN) {
            if (read(data_pipe[0], buffer, BUFSZ) < 0) {
                perror("read from data_pipe[0]");
                return -1;
            }
            crypto(buffer);
            if (write(STDOUT_FILENO, buffer, BUFSZ) < 0) {
                perror("write");
                return -1;
            }
        }
    }
    return 0;
}

int sender(pid_t pid, int* data_pipe)
{
    char buffer[BUFSZ] = {0}, sendbuf[BUFSZ + IDSZ] = {0};
    if (read(STDIN_FILENO, buffer, BUFSZ) < 0) {
        perror("read");
        return -1;
    }
    buffer[strlen(buffer) - 1] = '\0';
    if (snprintf(sendbuf, BUFSZ, "%s%s", id, buffer) < 0) {
        perror("sprintf");
        return -1;
    }
    if (strncmp(buffer, "broadcast", sizeof("broadcast") - 1) == 0) {
        if (broadcast_client() < 0) {
            perror("broadcast_client");
            return -1;
        }
    }
    else {
        crypto(sendbuf);
        if (sendto(sk, sendbuf, BUFSZ, 0, (struct sockaddr*)name, sizeof(*name)) < 0) {
            perror("sendto");
            return -1;
        }
        if (strncmp(buffer, "quit", sizeof("quit") - 1) == 0) {
            kill(pid, SIGTERM);
            printf("server disconnected\n");
            raise(SIGKILL);
        }
    }
    if (read_receiver(data_pipe) < 0) {
        perror("read_receiver");
        return -1;
    }
}

int receiver(struct sockaddr_in* hear, int* data_pipe)
{
    while(1) {
        char buffer_rec[BUFSZ] = {0};
        recvfrom(sk, buffer_rec, BUFSZ, MSG_DONTWAIT, (struct sockaddr*)hear, &(int){sizeof(*hear)});
        if (buffer_rec[0] == 0)
            break;
        if (write(data_pipe[1], buffer_rec, BUFSZ) < 0) {
            perror("write data_pipe[1]");
            return -1;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("argc < 2\n");
        exit(1);
    }
    struct sockaddr_in namet = {AF_INET, htons(PORT), 0}, hear = {AF_INET, 0, htonl(INADDR_ANY)};
    name = &namet;
    int ret, i = 0, data_pipe[2];
    pipe(data_pipe);
    char buffer[BUFSZ] = {0};
    if (inet_aton(argv[1], &name->sin_addr) == 0) {
        perror("Inputted IP-Adress");
        return -1;
    }
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        perror("socket sk");
        return -1;
    }
    if (gen_id(id) < 0) {
        perror("gen_id");
        return -1;
    }
    if (sprintf(buffer, "%s%s", "!connect!", id) < 0) {
        perror("sprintf");
        return -1;
    }
    if (bind(sk, (struct sockaddr*)&hear, sizeof(hear)) < 0) {
        perror("bind sk");
        return -1;
    }
    getsockname(sk, (struct sockaddr*)&hear, &(int){sizeof(hear)});
    printf("client receiver is bind on port %d\n", hear.sin_port);
    if (sendto(sk, buffer, strlen(buffer), 0, (struct sockaddr*)name, sizeof(*name)) < 0) {
        perror("sending first msg failed");
        return -1;
    }
    pid_t pid = fork();
    if (pid == 0) {
        while(1) {
            if (receiver(&hear, data_pipe) < 0) {
                perror("receiver");
                return -1;
            }
        }
    }
    signal(SIGINT, interrupted);
    while(1) {
        if (sender(pid, data_pipe) < 0) {
            perror("sender");
            return -1;
        }
    }
    return 0;
}