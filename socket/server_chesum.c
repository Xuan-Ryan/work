#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
char ch2[100];
int makeint(char s)
{
    int d;
    d = s — ‘0’;
    return d;
}
int checksum(char *s)
{
    //sum
    int i, j;
    int sum = 0;
    int c = 0;
    for (i = 0; i <= 3; i++)
    {
        c = 0;
        for (j = strlen(s) - 1; j >= 0; j = j - 4)
        {
            c += makeint(s[j - i]);
        }
        sum += (c << i);
    }
    int num[4] = {0, 0, 0, 0};
    //binary and complement
    int count = 3;
    while (sum != 0)
    {
        num[count] = sum % 2;
        sum = sum / 2;
        count — ;
    }
    sum = 0;
    for (i = 0; i < 4; i++)
    {
        num[i] = (num[i] + 1) % 2;
        sum += num[i];
    }
    return sum;
}
char *setsum(char *s)
{
    int len = strlen(s);
    int i = 0, j;
    char ch1[100];
    if (len % 4 != 0)
    {
        len = len % 4;
        for (i = 0; i < (4 - len); i++)
            ch1[i] = ‘0’;
    }
    for (i = 0; i < 100; i++)
        ch2[i] = ‘\0’;
    ch1[i] = ‘\0’;
    strcat(ch1, s);
    strcat(ch2, ch1);
    printf(“% s”, ch1);
    //sum
    int sum = 0;
    int c = 0;
    for (i = 0; i <= 3; i++)
    {
        c = 0;
        for (j = strlen(ch1) - 1; j >= 0; j = j - 4)
        {
            c += makeint(ch1[j - i]);
        }
        sum += (c << i);
    }
    int num[4] = {0, 0, 0, 0};
    //binary and complement
    int count = 3;
    while (sum != 0)
    {
        num[count] = sum % 2;
        sum = sum / 2;
        count — ;
    }
    printf(“\n”);
    char con[4];
    for (i = 0; i < 4; i++)
    {
        num[i] = (num[i] + 1) % 2;
        con[i] = num[i] + ‘0’;
        //printf(“%c”,con[i]);//ones complement and to char array
    }
    con[i] = ‘\0’;
    strcat(ch2, con);
    return ch2;
}
int main()
{
    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;
    int i;
    char buf[100];
    char s[100];
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf(“Cannot create socket exit\n”);
        exit(0);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(6001);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf(“Unable to bind local address”);
        exit(0);
    }
    listen(sockfd, 5);
    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            printf(“Accept Error \n”);
            exit(0);
        }
        //Message from server
        printf(“Server
               : “);
        scanf(“% s”, s);
        char *msg;
        msg = setsum(s);
        for (i = 0; i < 100; i++)
        {
            buf[i] = ‘\0’;
        }
        strcpy(buf, msg);
        send(newsockfd, buf, 100, 0);
        for (i = 0; i < 100; i++)
        {
            buf[i] = ‘\0’;
        }
        recv(newsockfd, buf, 100, 0);
        int d = checksum(buf);
        if (d == 0)
            printf(“No error ”);
        else
        {
            printf(“Error in the code”);
        }
        printf(“% s\n”, buf);
        close(newsockfd);
    }
}