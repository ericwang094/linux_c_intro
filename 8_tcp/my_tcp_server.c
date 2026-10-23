#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <unistd.h>

#define BUFFER_LENGTH       1024
#define EPOLL_SIZE          1024

void *client_routine(void *arg) {
    int clientfd = *(int *)arg;

    while(1) {
        char buffer[BUFFER_LENGTH] = {0};
        int len = recv(clientfd, buffer, BUFFER_LENGTH, 0);
        if (len < 0){ 
            close(clientfd);
            break;
        } else if(len == 0) {
            close(clientfd);
            break;
        } else {
            printf("Recv: %s, %d byte(s)\n", buffer, len);
        }
    }
    close(clientfd);
    return NULL;

}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Param Error\n");
        return -1;
    }

    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;


    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
        perror("bind error\n");
        return 2;
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen");
        return 3;
    }      // 5 means the maximum waiting incoming connection


#if 0

    while (1) {

        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        socklen_t client_len = sizeof(client_addr);

        int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);


        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_routine, &clientfd);

    }
#else
    int epfd = epoll_create(1); // as long as this param is greater than 0, it is ok, 1 has same meaning as 10000
    struct epoll_events events[EPOLL_SIZE] = {0};

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while(1) {
        int nready = epool_wait(epfd, events, EPOLL_SIZE, 5);
        if (nready == -1) continue;

        int i = 0;
        for (i = 0; i < nready; i++) {
            if (events[i].data.fd == sockfd) { //listen

                struct sockaddr_in client_addr;
                memset(&client_addr, 0, sizeof(struct sockaddr_in));
                socklen_t client_len = sizeof(client_addr);

                int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            } else {
                int clientfd = events[i].data.fd;        

				        char buffer[BUFFER_LENGTH] = {0};
                int len = recv(clientfd, buffer, BUFFER_LENGTH, 0);
                if (len < 0){ 
                    close(clientfd);

                    ev.events = EPOLLIN;
                    ev.data.fd = clientfd;
                    break;
                } else if(len == 0) {
                    close(clientfd);

                    ev.events = EPOLLIN;
                    ev.data.fd = clientfd;
                    break;
                } else {
                    printf("Recv: %s, %d byte(s)\n", buffer, len);
                }
            }
        }
    }
#endif
}

