#include "my_server.h"


int main(int argc, char* argv[])
{
    struct sockaddr_in name = {0};
    int sk, ret;
    char buffer[BUFSZ] = {0};
    sk = socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0)
    {
        perror("socket");
        exit(1);
    }
    name.sin_family = AF_INET;
    //strncpy(name.sun_path, PATH, sizeof(PATH));
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
        ret = write(sk, buffer, BUFSZ);
        if (ret < 0 || ret > BUFSZ)
        {
            perror("write");
            exit(1);
        }
    }
    return 0;
}