#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0
    servaddr.sin_port = htons(2000);
    int bind_result = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
    if (bind_result == -1) {
        printf("Bind failed: \n");
    }

    listen(sockfd, 10);

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr); // == sizeof(sockaddr_in);

    while (1) {

        printf("accept\n");
        int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len); // 这边会阻塞住
        printf("accept finished\n");

        char buffer[1024] = {0};
        int count = recv(clientfd, buffer, 1024, 0);  // 阻塞
        printf("Recv: %s\n", buffer);

        count = send(clientfd, buffer, 1024, 0);
        printf("listen finished: %d\n", count);
    }

    getchar(); // 阻塞

    printf("Exit\n");
    return 0;
}
