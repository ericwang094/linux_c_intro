#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXLINE 1024
#define LISTENNQ 1024

int main(int argc, char *argv[]) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        fprintf(stderr, "socket error");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr= htonl(INADDR_ANY); // listen to any
    servaddr.sin_port = htons(13);

    int bind_result = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (bind_result < 0) {
        fprintf(stderr, "bind failed");
    }

    int listen_return = listen(listenfd, LISTENNQ);
    if (listen_return < 0) {
        fprintf(stderr, "listen failed");
    }

    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        
        int write_return = write(connfd, buff, strlen(buff));
        if (write_return != strlen(buff)) {
            fprintf(stderr, "write failed");
        }

        close(connfd);
    }
}
