#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

    fd_set rfds;  // NOTE: 指fd的set的大小，集合大小？ 这个集合是什么东西
    fd_set rset;
    FD_ZERO(&rfds); // clean the rfds
    FD_SET(sockfd, &rfds); // NOTE: 把这个fd sets 设置（set） 到 sockfd上

    int maxfd = sockfd; // define the maximum value of the fd

    while (1) {
        rset = rfds;
        // param 1, the maximum loop value, if i have maxfx define as 9, totoal loop is 0-9=10 so maxfd + 1
        // param 2, the readfd, those fd are the fds that select will keep close look at to see if they can be read
        // param 3, the writefd, pay attention of fds that we want to write
        // nready return how many read and write available fd from 2 and 3
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL); // NOTE: 最后一个参数，如果没有client，就会一直等，然后阻塞

        if(FD_ISSET(sockfd, &rset)) {  // NOTE: 这个可读的rset是否在listen sock中被设置
            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            printf("accept finished: %d\n", clientfd);
            FD_SET(clientfd, &rfds);

            if (clientfd > maxfd) maxfd = clientfd; 
            // NOTE: 这里需要加判断，是因为 clientfd是会被回收。如果一个新的connection 用的是老的clientfd，maxfd维持不变的。
            // 只有在clientfd大于maxfd的时候，maxfd才等于clientfd
        }
        // recv/send -> clientfd
        int i = 0;
        for (i = sockfd + 1; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) {  // NOTE: 查看当前的 socket是不是可读
                char buffer[1024] = {};
                int count = recv(i, buffer, 1024, 0);

                if (count == 0) {  // recv return 0 means discconect
                    printf("client disconnect: %d\n", i);
                    close(i);
                    FD_CLR(i, &rfds);
                    continue;
                }

                count = send(i, buffer, 1024, 0);
                printf("SEND: %d\n", count);
            }
        }

    }

    getchar();
    printf("exit\n");
    return 0;
}

