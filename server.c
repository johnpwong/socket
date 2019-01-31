#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define CN 4 //connect num


int connfd[CN];		//save the connect sock id 
pthread_t thread[CN] = {0};	//save the thread id

struct sockinfo {	//send to routine func
	struct sockaddr_in addr;
	int index;
	int th_index;
};

void routine(struct sockinfo *info) {	//thread for read and write to client
	char *addrip = inet_ntoa(info->addr.sin_addr);
	int ind = info->index;
	int thind = info->th_index;

	char *buf = malloc(100);
	char *wrtbuf = malloc(100);
	
	sprintf(wrtbuf, "[system] : %s online!\n", addrip);
	printf("%s", wrtbuf);

	for(int i=0; i<CN; i++) {	//broadcast
		if(connfd[i]!=-1)
		{
			write(connfd[i], wrtbuf, strlen(wrtbuf));
		}
	}
	
	while(1) {
		bzero(buf, 100);
		bzero(wrtbuf, 100);
		
		if(read(connfd[ind], buf, 100) == 0) {
			break;
		}
		
		sprintf(wrtbuf, "[%s] : %s", addrip, buf);
		printf("%s", wrtbuf);

		for(int i=0; i<CN; i++) {	//broadcast
			if(connfd[i]!=-1 && i!=ind) 
				write(connfd[i], wrtbuf, strlen(wrtbuf));
		}
	}
	
	sprintf(wrtbuf, "[system] : %s offline!\n", addrip);	//offline broadcast
	printf("%s", wrtbuf);
	
	for(int i=0; i<CN; i++) {
		if(connfd[i]!=-1)
			write(connfd[i], wrtbuf, strlen(wrtbuf));
	}
	
	connfd[ind] = -1;
	thread[thind] = 0;	
}


int main(int argc, char *argv[])
{
	for(int i=0; i<CN; i++) {
		connfd[i] = -1;
	}

	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1) {
		perror("socket");
		exit(-1);
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[1]));
	bind(sfd, (struct sockaddr *)&addr, sizeof(addr));
	
	listen(sfd, CN);

	struct sockaddr_in cli_addr;
	socklen_t len = sizeof(cli_addr);

	struct sockinfo info;
	while(1) {
		bzero(&cli_addr, len);

		int index;
		for(index=0; index<CN; index++) {
			if(connfd[index]==-1) {
				break;
			}
		}
		
		connfd[index] = accept(sfd, (struct sockaddr *)&cli_addr, &len);
		
		bzero(&info, len);
		info.addr = cli_addr;
		info.index = index;
		
		for(int i=0; i<CN; i++) {
			if(thread[i]==0) {
				info.th_index = i;
				pthread_create(&thread[i], NULL, (void *)routine, &info); 
				break;
			}
		}
	}
	
	
	return 0;

}

