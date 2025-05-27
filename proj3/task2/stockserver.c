/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

void echo(int connfd);

struct item {
	int ID;
	int left_stock;
	int price;
	int readcnt;
	sem_t mutex;
};


// Recommendation: 먼저 아래와 같이 command string을 server가 concurrency하게 판독할 수 있게 구현
// Stock.txt 읽어서 binary tree 생성
//

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
    }

    listenfd = Open_listenfd(argv[1]);

	while (1) {
		clientlen = sizeof(struct sockaddr_storage);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,  client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
		echo(connfd);
		Close(connfd);
    }
    exit(0);
}
/* $end echoserverimain */
