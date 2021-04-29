#include "my_server.h"
static int sk;
static const long double g = 23, p = 256, b = 11, a = 4;
static char K;
void interrupted(int signum)
{
    if (write(sk, "quit", BUFSZ) < 0) {
        perror("write");
    }
    exit(0);
}


int receiver()
{
    char buffer[BUFSZ] = {0};
    if (read(STDIN_FILENO, buffer, BUFSZ) < 0 ) {
        perror("read");
        return -1;
    }
    crypto(buffer);
    if (write(sk, buffer, BUFSZ) < 0) {
        perror("write");
        return -1;
    }
    crypto(buffer);
    if (strncmp(buffer, "quit", sizeof("quit") - 1) == 0) {
        printf("server disconnected\n");
        exit(0);
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
        else {
            perror("!POLLIN");
            return -1;
        }
    }
}

void terminal_settings()
{
    struct termios tattr;
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

int main(int argc, char* argv[])
{
    terminal_settings();
    char B = gen_open_key_client(g, b, p);
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
    char temp[1];
    temp[0] = B;
    if (write(sk, temp, 1) < 0) { //send fist msg to server
        perror("write hello msg");
        return -1;
    }
    if (read(sk, temp, 1) < 0) {
        perror("read");
        return -1;
    }
    char A = temp[0];
    //printf("A, B is %d %d\n", A, B);
    K = gen_close_key_client(A, b, p);
    signal(SIGINT, interrupted);
    while(1) {
        if (receiver() < 0) {
            perror("receiver");
            return -1;
        }
    }
    return 0;
}