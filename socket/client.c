#include "my_server.h"


int main()
{
    struct sockaddr_un name = {0};
    int sk, ret;
    char buffer[BUFSZ] = {0};
    sk = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sk < 0)
    {
        perror("socket");
        exit(1);
    }
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, PATH, sizeof(PATH));
    ret = connect(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("connect");
        close(sk);
        return 1;
    }
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
    return 0;
}