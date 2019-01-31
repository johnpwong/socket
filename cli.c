#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

int sfd;

void routine() //read messages from server 
{	
	char *buf = malloc(100);
	while(1)
	{
		bzero(buf, 100);
		if(read(sfd, buf, 100) == -1)
			break;
		printf("%s", buf);
	}
}

int main(int argc, char *argv[])
{
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1) {
		perror("socket");
		exit(-1);
	}

	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	srv_addr.sin_port = htons(atoi(argv[2]));

	connect(sfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

	pthread_t t;
	pthread_create(&t, NULL, (void *)routine, NULL);

	char *buf = malloc(100);
	while(1) {		//write to server
		bzero(buf, 100);
		if(fgets(buf, 100, stdin) == NULL)
			break;

		write(sfd, buf, strlen(buf));
	}

	
	close(sfd);

	return 0;

}

