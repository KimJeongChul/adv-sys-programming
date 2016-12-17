#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

#define IP "127.0.0.1"
#define MAX_DATA 1024
#define MAX_EVENTS 20
#define MAX_CLIENT 2

int num_client = 0;
int clientSocket[20];

int setNonBlocking(int socket)
{
	int flag;
	if((flag = fcntl(socket, F_GETFL, 0)) < 0 ) {
		perror("fcntl");
		return -1;
	}
	
	if((fcntl(socket, F_SETFL, flag | O_NONBLOCK)) < 0) { // NON-BLOCK mode add;
		perror("fcntl");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{	
	int acceptedSock;
	struct sockaddr_in Addr;
	socklen_t AddrSize = sizeof(Addr);
	char data[MAX_DATA], *p;
	int listenSock;
	int count, i = 1;
	char* port;
	
	int flag;
	struct epoll_event ev; 
	struct epoll_event *events;
	int nfds, epollfd;

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s;
	int j;

	int finishClient = 0;
	int boolCheckDollar = 1;

	if(argc != 2)
	{
		fprintf(stderr, "Usage : %s [port]\n", argv[0]);
		exit(0);
	}
	port = argv[1];

	memset(&hints, 0, sizeof(struct addrinfo));			

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	s = getaddrinfo(NULL, port , &hints, &result);
	if(s != 0) {
		perror("getaddrinfo");
		goto leave;
	}

	for(rp = result; rp != NULL; rp = rp->ai_next) {
		listenSock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(listenSock == -1)
			continue;

		s = bind(listenSock, rp->ai_addr, rp->ai_addrlen);
		if(s == 0)
			break;

		close(listenSock);
	}

	if(rp == NULL) {
		perror(	"Not Bind");
		goto leave;
	}	
	
	freeaddrinfo(result);
	
	s = setNonBlocking(listenSock);
	if(s == -1) {
		perror("Non Blocking");
		abort();
	}

	if ((listen(listenSock, SOMAXCONN)) < 0 ) {
 		perror("listen");
 		goto error;
	}	

	printf("listen\n");

	epollfd = epoll_create(MAX_EVENTS);
	if(epollfd == -1) {
		perror("epoll_create");
		goto error;
	}	

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = listenSock;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenSock, &ev) == -1) {
		perror("epoll_ctl :: listen socket");
		goto error;
	}

	events = calloc(MAX_EVENTS, sizeof ev);
	
	for(;;) {
		int n;
		char clientMsg[MAX_DATA];
		if((nfds = epoll_wait(epollfd, events, MAX_EVENTS, 1000)) == -1) {
			perror("epoll_wait");
			goto error;
		}

		//printf("number of num_client : %d\n",num_client);
		
		for(n = 0; n < nfds; n++) {
			/*
			if((events[n].events & EPOLLERR) || (events[n].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
				perror("epoll error");
				close(events[n].data.fd);
				continue;
			}*/
			if(events[n].data.fd == listenSock) {
				// accept
				while(1) {
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					struct sockaddr in_addr;
					socklen_t in_len;
					in_len = sizeof in_addr;
					if((acceptedSock = accept(listenSock, &in_addr, &in_len)) < 0) {
						if((errno == EAGAIN) || (errno == EWOULDBLOCK))
							break;
						else{ 
							perror("accept");
							break;
						}
					}
					
					s = getnameinfo (&in_addr, in_len, 
							hbuf, sizeof hbuf , 
							sbuf, sizeof sbuf , 
							NI_NUMERICHOST | NI_NUMERICSERV);
					
					if(s == 0)
						printf("Accepted connection on desriptor %d host=%s, port=%s\n",acceptedSock,hbuf,sbuf);
					
					clientSocket[num_client++] = acceptedSock;
					
					setNonBlocking(acceptedSock); // set non-block mode
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = acceptedSock; 

					if(epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptedSock, &ev) == -1) {
						perror("epoll_ctl :: accpted socket");
						goto error;
					}

					if(num_client == MAX_CLIENT) {
						if(boolCheckDollar) {
							for(j = 0; j < num_client; j++) {
								send(clientSocket[j], "$", 2, 0);
								printf("send client %d $\n",j);
							}
							boolCheckDollar = 0;
						} 
					}
				}
				continue;	
			}
			else {
				printf("finishClient : %d\n",finishClient);
				for(j = 0; j < num_client; j++) {
					recv(clientSocket[j], clientMsg, MAX_DATA, 0);
					printf("%s",clientMsg);
					int k;
					for(k = 0; k < num_client; k++) {
						send(clientSocket[k], clientMsg, strlen(clientMsg), 0);
					}
					if(!strcmp(clientMsg,"@")) {
						printf("finish client %d\n,",j);
						finishClient++;
					}
				}
				if(finishClient == MAX_CLIENT) {			
					for(j = 0; j < num_client; j++) {
						send(clientSocket[j], "%", 2, 0);
						printf("send client %d %\n",j);
						close(clientSocket[j]);
					}
				}
			}
		}
	}
error:
	exit(0);
leave:
	exit(0);
}

