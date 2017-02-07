#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define FILE "/dev/tty"
#define MAX_BUF 1024

void error(char *err_msg)
{
	perror(err_msg);
	exit(1);
}

// This function checks if a port was selected in the argc
int chk_portno(int *argc)
{
	if (*argc < 2)
	{
		fprintf(stderr, "%s\n", "ERROR: indicate the port number!");
		exit(1);
	}

	return 0;
}

void send_message(int* sockfd , char* buffer)
{
		scanf("%s", buffer);
		write(*sockfd, buffer, strlen(buffer));
}

int create_socket()
{
    return socket(PF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in create_serv_addr(char* argv[])
{
	struct sockaddr_in serv_addr;
   	serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;

    return serv_addr;
}

int main(int argc, char* argv[])
{
    chk_portno(&argc);
    char buffer[MAX_BUF];

	int sockfd = create_socket();
    struct sockaddr_in serv_addr = create_serv_addr(argv);
    //TODO  Necesito crear un thread que separe el envio y recepcion de mensajes 
	while(1)
	{
		// Intento conectarme a un servidor que este escuchando conexiones.
		if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) >= 0)
		{
			while(1)
			{
				// Si lo logro empezamos la comunicacion :D
				send_message(&sockfd, buffer);
				memset(buffer, 0, MAX_BUF);
				read(sockfd, buffer, MAX_BUF);
			}
		}
		else
		{
			// Si el cliente no logra contectarse, espera 5 segundos y vuelve a intentar
			sleep(5);
		}
	}
}
