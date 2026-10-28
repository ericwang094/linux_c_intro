
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>


void *client_thread(void *arg) {

    int clientfd = *(int *)arg;  // NOTE: fd is int, increment, starting from 3. 0 is stdin, 1 is stdout, 2 is stderr

    while (1) {

        char buffer[1024] = {0};
        int  count        = recv(clientfd, buffer, 1024, 0);
        printf("Recv %s\n", buffer);

        if (count == 0) {  // recv return 0 means discconect
            printf("client disconnect: %d\n", clientfd);
            close(clientfd);
            break;
        }

        count = send(clientfd, buffer, 1024, 0);
        printf("SEND: %d\n", count);
    }
}

int main(void) {
    int                sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0
    servaddr.sin_port        = htons(2000);       // 0-1023
    if (-1 == bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr))) {
        printf("Bind failed: %s\n", strerror(errno));
    }

    listen(sockfd, 10);
    printf("listen finished: %d \n", sockfd);

    struct sockaddr_in clientaddr;
    socklen_t          len = sizeof(clientaddr);

    int epfd = epoll_create(1);  // NOTE: 只要这个参数不是0 就行，因为之前的实现是数组，现在是链表
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while(1) {
        struct epoll_event events[1024] = {0};
        int nready = epoll_wait(epfd, events, 1024, -1);  // NOTE: -1 一直阻塞 

        int i = 0;
        for (i = 0; i < nready; i++) {
            int connfd = events[i].data.fd;
            if (connfd == sockfd) {
                int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
                printf("accept finished: %d\n", clientfd);

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            } else if (events[i].events & EPOLLIN) {
                char buffer[1024] = {0};

                int count = recv(i, buffer, 1024, 0);
                if (count == 0) {
                    printf("client disconnect: %d\n", connfd);
                    close(i);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, &ev);
                    continue;
                }

                printf("RECV: %s\n", buffer);
                count = send(i, buffer, count, 0);
                printf("SEND: %d\n", count);

            }
        }
    }


    getchar();
    printf("exit\n");
    return 0;
}

