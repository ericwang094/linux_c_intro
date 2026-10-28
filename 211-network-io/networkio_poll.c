#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

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

    struct pollfd fds[1024] = {0};
    fds[sockfd].fd = sockfd;
    fds[sockfd].events = POLLIN;
    int maxfd = sockfd;
    while (1) {
        int nready = poll(fds, maxfd + 1, -1);  // poll 把 fd copy 到 内核去，然后看fd是不是ready
        int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
        printf("accept finished: %d\n", clientfd);

        fds[clientfd].fd = clientfd;
        fds[clientfd].events = POLLIN;

        if (clientfd > maxfd) maxfd = clientfd;

        int i = 0;
        for (i = sockfd + 1; i <= maxfd; i++) {
            if (fds[i].revents & POLLIN) {
                char buffer[1024] = {};
                int count = recv(i, buffer, 1024, 0);

                if (count == 0) {  // recv return 0 means discconect
                    printf("client disconnect: %d\n", i);
                    close(i);

                    fds[i].fd = -1;
                    fds[i].events = 0;
                    continue;
                }

                printf("Recv: %s\n", buffer);
                count = send(i, buffer, 1024, 0);
                printf("SEND: %d\n", count);
            }
        }
    }
    getchar();
    printf("exit\n");
    return 0;
}
