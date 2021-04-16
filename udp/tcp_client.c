#include "my_server.h"


int main(int argc, char* argv[])
{
    struct sockaddr_in name = {0};
    int sk, ret;
    char port_str[BUFSZ] = {0};
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
    if (inet_aton(argv[1], &name.sin_addr) == 0) {
        perror("Inputted IP-Adress");
        exit(1);
    }
    ret = connect(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("connect");
        close(sk);
        return 1;
    }
    write(sk, "hello", sizeof("hello"));
    //sleep(1);
    read(sk, port_str, BUFSZ);
    printf("%s\n", port_str);
    close(sk);
    name.sin_port = (atoi(port_str));
    sk = socket(AF_INET, SOCK_STREAM, 0);
    ret = connect(sk, (struct sockaddr*)&name, sizeof(name));
    if (ret < 0)
    {
        perror("connect");
        close(sk);
        return 1;
    }
    while(1) {
        ret = read(STDIN_FILENO, buffer, BUFSZ);
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

        if (strncmp(buffer, "quit", sizeof("quit") - 1) == 0) {
            printf("buffer: %s\n", buffer);
            //kill(pid, SIGTERM);
            printf("server disconnected\n");
            exit(0);
        }
        memset(buffer, 0, BUFSZ);
        int retread = 1;
        while(retread > 0) {
            retread = read(sk, buffer, BUFSZ);
            if (retread < 0 || ret > BUFSZ)
            {
                perror("read");
                exit(1);
            }
            ret = write(STDOUT_FILENO, buffer, BUFSZ);
            if (ret < 0 || ret > BUFSZ)
            {
                perror("write");
                exit(1);
            }
            memset(buffer, 0, BUFSZ);
        }
    }
    return 0;
}