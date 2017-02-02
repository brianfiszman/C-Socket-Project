#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define MAX_BUF 1024

void error(char *err_msg)
{
	perror(err_msg);
	exit(1);
}

// This function checks if a port was selected in the argc
void chk_portno(int *argc)
{
	if (*argc < 2)
	{
		fprintf(stderr, "%s\n", "ERROR: indicate the port number!");
		exit(1);
	}
}

void send_message(int* sockfd , char* buffer){
		write(*sockfd, buffer, strlen(buffer));
}

int main(int argc, char* argv[])
{
	int sockfd;
	char buffer[MAX_BUF] = "hello";
	struct sockaddr_in serv_addr;
	
	chk_portno(&argc);

	serv_addr.sin_port 	 = htons(atoi(argv[1]));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
	sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) >= 0)	send_message(&sockfd, buffer);
	
	return 0;
}

