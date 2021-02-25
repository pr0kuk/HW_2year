#include "my_server.h"


int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("argc < 2\n");
        exit(1);
    }
    struct sockaddr_in name = {AF_INET, htons(23456), 0};
    struct sockaddr_in ans = {AF_INET, 0, 0};
    int sk, ret, ans_sk, i = 0;
    pid_t pid = getpid();
    char pid_str[MAX_PID];
    char path[PATH_MAX];
    char serv_ip[1];
    if (inet_aton(argv[1], &name.sin_addr) == 0) {
        perror("Inputted IP-Adress");
        exit(1);
    }
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0)
    {
        perror("socket sk");
        exit(1);
    }
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
    ret = sprintf(pid_str, "%s%d", "!connect!", pid);
    if (ret < 0) {
        perror("sprintf");
        exit(1);
    }
    ret = sendto(sk, pid_str, strlen(pid_str), 0, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("sending first msg failed");
        exit(1);
    }



    while(1) {
        char buffer[BUFSZ] = {0};
        char sendbuf[BUFSZ + IDSZ] = {0};
        ret = read(1, buffer, BUFSZ);
        if (ret < 0 || ret > BUFSZ) 
        {
            perror("read");
            exit(1);
        }
        buffer[strlen(buffer)-1] = 0;
        ret = sprintf(sendbuf, "%d!%s", pid, buffer);
        if (ret < 0)
            perror("sprintf");
        if (strncmp(buffer, "exit", sizeof("exit") - 1) == 0) {
            printf("Disconnected..\n");
            exit(0);
        }
        if (strncmp(buffer, "broadcast", sizeof("broadcast") - 1) == 0) {
            printf("Broadcasting..\n");
            ans.sin_addr.s_addr = INADDR_BROADCAST, ans.sin_port = htons(23456);
            ret = sendto(ans_sk, "!hello!", sizeof("!hello!")-1, 0, (struct sockaddr*)&ans, sizeof(ans));
            if (ret < 0 || ret > sizeof("!hello!")-1) {
                perror("broadcast");
                exit(1);
            }
            ans.sin_addr.s_addr = htonl(INADDR_ANY), ans.sin_port = 0;
            ret = bind(ans_sk, (struct sockaddr*)&ans, sizeof(ans));
            if (ret < 0) {
                perror("bind"); //It is printed EINVAL, I don't know why
                //exit(1);
            }
            printf("Waiting for answers (%d clocks)..\n", CLOCKS_TO_WAIT);
            for (i = 0; i < CLOCKS_TO_WAIT; i++) {
                ret = recvfrom(ans_sk, serv_ip, sizeof(serv_ip), MSG_DONTWAIT, (struct sockaddr*)&ans, &(int){sizeof(ans)});
                if (ret >= 0)
                    printf("%s server found\n", inet_ntoa(ans.sin_addr));
            }
            printf("Waiting is over..\n");
        }
        else {
            ret = sendto(sk, sendbuf, BUFSZ + IDSZ, 0, (struct sockaddr*)&name, sizeof(name));
            if (ret < 0 || ret > BUFSZ + IDSZ) {
                perror("write");
            }
        }
    }
    return 0;
}