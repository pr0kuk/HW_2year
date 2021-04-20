#include "my_server.h"
static int sk;

void interrupted(int signum)
{
    int ret = write(sk, "quit\n", BUFSZ);
    if (ret < 0 || ret > BUFSZ) {
        perror("write");
        exit(1);
    }
    raise(SIGKILL);
}

void cypher()
{
    return;
}

int receiver()
{
    char* buffer[BUFSZ] = {0};
    int ret = read(STDIN_FILENO, buffer, BUFSZ);
    if (ret < 0 || ret > BUFSZ) {
        perror("read");
        exit(1);
    }
    ret = write(sk, buffer, BUFSZ);
    if (ret < 0 || ret > BUFSZ) {
        perror("write");
        exit(1);
    }
    if (strncmp(buffer, "quit", sizeof("quit") - 1) == 0) {
        printf("server disconnected\n");
        exit(0);
    }
    struct pollfd pollfds = {sk, POLLIN};
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        if (memset(buffer, 0, BUFSZ) == NULL)
            perror("memset");
        if (pollfds.revents == POLLIN) {
            ret = read(sk, buffer, BUFSZ);
            if (ret < 0) {
                perror("read from sk");
                exit(1);
            }
            ret = write(STDOUT_FILENO, buffer, BUFSZ);
            if (ret < 0 || ret > BUFSZ) {
                perror("write");
                exit(1);
            }
        }
    }
}



int main(int argc, char* argv[])
{
    struct sockaddr_in name = {AF_INET, htons(PORT), 0};
    int ret;
    char port_str[BUFSZ] = {0};
    char buffer[BUFSZ] = {0};
    sk = socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        perror("socket");
        exit(1);
    }
    if (inet_aton(argv[1], &name.sin_addr) == 0) {
        perror("Inputted IP-Adress");
        exit(1);
    }
    if (connect(sk, (struct sockaddr*)&name, sizeof(name)) < 0) {
        perror("connect");
        close(sk);
        return 1;
    }
    if (write(sk, "hello", sizeof("hello")) < 0) { //send fist msg to server
        perror("write hello");
        return -1;
    }
    if (read(sk, port_str, BUFSZ) < 0) {
        perror("read port");
        return -1;
    }
    printf("port of my server subprocess: %s\n", port_str);
    if (close(sk) < 0) {
        perror("close");
        return -1;
    }
    name.sin_port = (atoi(port_str));
    sk = socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        perror("socket");
        exit(1);
    }
    if (connect(sk, (struct sockaddr*)&name, sizeof(name)) < 0) {
        perror("connect");
        close(sk);
        return 1;
    }
    signal(SIGINT, interrupted);
    while(1)
        receiver();
    return 0;
}