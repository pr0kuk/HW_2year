#include "my_server.h"
int main()
{
    int sk, ret;
    struct sockaddr_un name = {0};
    sk = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sk < 0)
    {
        perror("socket");
        return 1;
    }
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, PATH, sizeof(PATH));

    ret = bind(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("bind");
        close(sk);
        return 1;
    }
    ret = listen(sk, 20);
    if (ret)
    {
        perror("listen");
        close(sk);
        return 1;
    }
    while (1)
    {
        int client_sk;
        char buffer[BUFSZ] = {0};
        client_sk = accept(sk, NULL, NULL);
        if (client_sk < 0)
        {
            perror("accept");
            exit(1);
        }
        ret = read(client_sk, buffer, BUFSZ);
        if (ret < 0 || ret > BUFSZ)
        {
            perror("read");
            exit(1);
        }
        close(client_sk);
        if (strncmp(buffer, "print", sizeof("print")-1) == 0)
            printf("%s", buffer+sizeof("print"));
        if (strncmp(buffer, "exit", sizeof("exit")-1) == 0)
            break;
    }
    unlink(PATH);
}