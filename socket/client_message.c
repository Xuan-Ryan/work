//傳文字訊息
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{

    //socket的建立
    int sockfd = 0;
    int i;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        printf("Fail to create a socket.");
    }

    //socket的連線

    struct sockaddr_in info; //對方的資訊
    bzero(&info, sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("192.168.1.76");
    info.sin_port = htons(8080);

    int err = connect(sockfd, (struct sockaddr *)&info, sizeof(info));
    if (err == -1)
    {
        printf("Connection error");
    }

    //Send a message to server
    char message[] = {"Hi there"};
    char receiveMessage[100] = {};
    for (i = 0; i <= 10; i++)
    {
        send(sockfd, message, sizeof(message), 0);
        // recv(sockfd, receiveMessage, sizeof(receiveMessage), 0);
        //printf("%s", receiveMessage);

        //bzero(receiveMessage, sizeof(receiveMessage));
        usleep(500000); //傳太快收不到
        printf("%s\n", message);
    }

    printf("close Socket\n");

    close(sockfd);
    return 0;
}
