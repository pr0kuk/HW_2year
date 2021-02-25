#include "my_server.h"


int main(int argc, char* argv[])
{
    struct sockaddr_in name = {0};
    int sk, ret;
    char buffer[BUFSZ] = {0};
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    char path[MAX_PATH];
    if (sk < 0)
    {
        perror("socket");
        exit(1);
    }
    name.sin_family = AF_INET;
    name.sin_port = htons(23456);
    name.sin_addr.s_addr = inet_addr(argv[1]);
    ret = connect(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("connect");
        close(sk);
        return 1;
    }
    while(1) {
        ret = read(1, buffer, BUFSZ);
        if (ret < 0 || ret > BUFSZ)
        {
            perror("read");
            exit(1);
        }
        if (strncmp(buffer, "cd", sizeof("cd") - 1) == 0) {
            buffer[strlen(buffer)-1] = 0;
            strcpy(path, buffer);
            continue;
        }
        ret = sendto(sk, buffer, BUFSZ, 0, (struct sockaddr*)&name, sizeof(name));
        if (ret < 0 || ret > BUFSZ)
        {
            
            perror("write");
            exit(1);
        }
    }
    return 0;
}