#include "my_server.h"
static int sk;

void interrupted(int signum)
{
    if (write(sk, "quit", BUFSZ) < 0) {
        perror("write");
    }
    raise(SIGKILL);
}


int receiver()
{
    char buffer[BUFSZ] = {0};
    if (read(STDIN_FILENO, buffer, BUFSZ) < 0 ) {
        perror("read");
        return -1;
    }
    buffer[strlen(buffer) - 1] = '\0';
    crypto(buffer);
    if (write(sk, buffer, BUFSZ) < 0) {
        perror("write");
        return -1;
    }
    crypto(buffer);
    if (strncmp(buffer, "quit", sizeof("quit") - 1) == 0) {
        printf("server disconnected\n");
        raise(SIGKILL);
    }
    struct pollfd pollfds = {sk, POLLIN};
    while (poll(&pollfds, 1, POLL_WAIT) != 0) {
        memset(buffer, 0, BUFSZ);
        if (pollfds.revents == POLLIN) {
            if (read(sk, buffer, BUFSZ) < 0) {
                perror("read from sk");
                return -1;
            }
            crypto(buffer);
            if (write(STDOUT_FILENO, buffer, BUFSZ) < 0) {
                perror("write");
                return -1;
            }
        }
    }
}



int main(int argc, char* argv[])
{
    struct sockaddr_in name = {AF_INET, htons(PORT), 0};
    char buffer[BUFSZ] = {0};
    sk = socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        perror("socket");
        return -1;
    }
    if (inet_aton(argv[1], &name.sin_addr) == 0) {
        perror("Inputted IP-Adress");
        return -1;
    }
    if (connect(sk, (struct sockaddr*)&name, sizeof(name)) < 0) {
        perror("connect");
        close(sk);
        return -1;
    }
    if (write(sk, "hello", sizeof("hello")) < 0) { //send fist msg to server
        perror("write hello");
        return -1;
    }
    signal(SIGINT, interrupted);
    while(1) {
        if (receiver() < 0) {
            perror("receiver");
            return -1;
        }
    }
    return 0;
}