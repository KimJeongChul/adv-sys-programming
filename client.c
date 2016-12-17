#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>

#define IP "127.0.0.1"
#define MAX_CLIENT 1024
#define BUFF 1024
#define PATH 30
#define PORT 9002

int sock;

void *clientRead(void *arg); // client file read and send
void *clientWrite(void *arg); // client file write and recv

int main(int argc, char *argv[])
{
	int clientSock;
	struct sockaddr_in serverAddr;
	fd_set rfds, wfds, efds;
	int ret = -1;
	char rdata[BUFF];
	int i = 1;
	struct timeval tm;
	//pthread
	pthread_t clientThread[2];
	int threadIdRead;
	int threadIdWrite;
	int fileNo;
	int status;

	if(argc != 2) {
		fprintf(stderr, "usage : %s number\n", argv[0]);
	}
	
	fileNo = atoi(argv[1]);
	if ((ret = clientSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		goto leave;
	}

	printf("socket() : create a socket\n");

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(IP);
	serverAddr.sin_port = htons(PORT);

	if ((ret = connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)))) {
 		perror("connect");
 		goto leave1;
	}else {
		printf("[CLIENT] Connected to %s\n", inet_ntoa(*(struct in_addr *)&serverAddr.sin_addr));
		send(clientSock,"!",2,0);	
	}
	sock = clientSock;
	int flag;
	flag = fcntl(ret, F_GETFL, 0);
	fcntl(ret,F_SETFL, flag | O_NONBLOCK);

	ioctl(0, FIONBIO, (unsigned long *)&i);
	if((ret = ioctl(clientSock, FIONBIO, (unsigned long *)&i))) {
		perror("ioctl socket");
		goto leave;
	}
	
	tm.tv_sec = 1; tm.tv_usec = 1000;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);	

	while (1) {
		FD_SET(clientSock, &rfds);
		FD_SET(clientSock, &efds);
		FD_SET(clientSock, &wfds);

		if ((ret = select(clientSock+1, &rfds, &wfds, &efds, &tm)) < 0) {
			perror("select");
			goto leave1;
		}else if (!ret) {	// nothing happened within tm
			continue;
		}
					

		if (FD_ISSET(clientSock, &efds)) {
 			printf("Connection closed\n");
 			goto leave1;
 		}
		if (FD_ISSET(clientSock, &rfds)) {
			if((ret = recv(clientSock, rdata, BUFF, 0)) < 0 ) {
				printf("Connection closed by remote host\n");
				exit(1);
			}	
			//printf("%s\n",rdata);
			if(!strcmp(rdata,"$")) {			
				threadIdRead = pthread_create(&clientThread[0], NULL, clientRead, (void*)&fileNo);
				if(threadIdRead < 0) {
					perror("thread create error");
					exit(1);
				}
				pthread_join(clientThread[0], (void**)&status);
				printf("return thread %d %d\n", fileNo, status);
			}else if(!strcmp(rdata,"%")){
				printf("close socekt\n");
				close(ret);	
			}else {
				//printf("%s\n",rdata);
				threadIdWrite = pthread_create(&clientThread[1],NULL, clientWrite, (void*)&fileNo);
				if(threadIdWrite < 0) {
					perror("thread create error");
					exit(1);
				}
				pthread_join(clientThread[1], (void**)&status);
				//printf("return thread %d %d\n", fileNo, status);				
			}
		}
		if(FD_ISSET(clientSock, &wfds)) {
			;
		}
	}
leave1:
	close(clientSock);
leave:
	return -1;
}

	
void *clientRead(void *arg) // client file read and send
{
	FILE *fin;
	char fileBuff[BUFF];
	char dirPath[PATH];
	char strFileNo[2];
	int fileNo;
	
	strcpy(dirPath,"/home/pi/workspace/adv-sys-programming/file_");
	
	fileNo = *((int*)arg);

	sprintf(strFileNo, "%d", fileNo);		
	strcat(dirPath, strFileNo);
	
	if((fin = fopen(dirPath, "r")) == NULL) {
		perror(dirPath);
		exit(0);
	}
	
	while(!feof(fin)) {
		fgets(fileBuff, BUFF, fin);
		//printf("%s\n",fileBuff);
		send(sock, fileBuff, strlen(fileBuff), 0);		
	}	
        send(sock, "@", 2, 0);	
	printf("send @ to server\n");
	fclose(fin);
	
}

void *clientWrite(void *arg) // client file write and recv
{
	FILE *fout;
	char fileBuff[BUFF];
	char dirPath[PATH];
	char strFileNo[2];
	int fileNo;
	
	strcpy(dirPath,"/home/pi/workspace/adv-sys-programming/client-file/file_");
	
	fileNo = *((int*)arg);
	
	sprintf(strFileNo, "%d", fileNo);
	strcat(dirPath, strFileNo);

	if((fout = fopen(dirPath, "a")) == NULL) {
		perror(dirPath);
		exit(0);
	}
	
	recv(sock, fileBuff, BUFF, 0);	
	//printf("%s",fileBuff);
	fputs(fileBuff, fout);	
	fclose(fout);
}
