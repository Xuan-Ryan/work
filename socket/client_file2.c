//////////////////////////////////////////////////////
// file_client.c  socket傳輸檔案client程式
// 寫入檔案時判斷數量 改善之後傳送的封包
//優化
//MD5 未完成(尚未編碼對照)
/////////////////////////////////////////////////////
//利用外部輸入IP address
//port 9000

#include <stdio.h>      // for printf
#include <stdlib.h>     // for exit
#include <string.h>     // for bzero
#include <sys/types.h>  // for socket
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <openssl/md5.h>

#define KGRN "\033[0;32;32m"
#define RESET "\033[0m"

#define HELLO_WORLD_SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

char *printf_md5(unsigned char *in)
{
    int i;
    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        if (in[i] < 0x10)
            printf("0%1x", in[i]);
        else
            printf("%2x", in[i]);
    }

    printf("\n" RESET);
    return in;
}

int main(int argc, char *argv[])
{
    int sum = 0;

    if (argc != 2)
    {
        printf("Usage: ./%s ServerIPAddress\n", argv[0]);
        exit(1);
    }

    // 設定一個socket地址結構client_addr, 代表客戶機的internet地址和埠
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;                // internet協議族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY); // INADDR_ANY表示自動獲取本機地址
    client_addr.sin_port = htons(0);                 // auto allocated, 讓系統自動分配一個空閒埠

    // 建立用於internet的流協議(TCP)型別socket,用client_socket代表客戶端socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    // 把客戶端的socket和客戶端的socket地址結構繫結
    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n");
        exit(1);
    }

    // 設定一個socket地址結構server_addr,代表伺服器的internet地址和埠
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    // 伺服器的IP地址來自程式的引數
    if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }

    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);

    // 向伺服器發起連線請求,連線成功後client_socket代表客戶端和伺服器端的一個socket連線
    if (connect(client_socket, (struct sockaddr *)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n", argv[1]);
        exit(1);
    }

    unsigned char file_name[FILE_NAME_MAX_SIZE + 1];
    bzero(file_name, sizeof(file_name));
    printf("Please Input File Name On Server.\t");
    scanf("%s", file_name);

    unsigned char buffer[BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));
    strncpy(buffer, file_name, strlen(file_name) > BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));

    // 向伺服器傳送buffer中的資料,此時buffer中存放的是客戶端需要接收的檔案的名字
    send(client_socket, buffer, BUFFER_SIZE, 0);

    FILE *fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        printf("File:\t%s Can Not Open To Write!\n", file_name);
        exit(1);
    }

    long Packlength = 0;
    char receiveMessage[16] = {};
    recv(client_socket, receiveMessage, sizeof(receiveMessage), 0);
    printf("Packlength : %s\n", receiveMessage);

    sscanf(receiveMessage, "%ld", &Packlength);
    printf("integer Packlength : %ld\n", Packlength);

    // 從伺服器端接收資料到buffer中
    bzero(buffer, sizeof(buffer));
    int length = 0;

    while (length = recv(client_socket, buffer, BUFFER_SIZE, 0))
    {
        if (length < 0)
        {
            printf("Recieve Data From Server %s Failed!\n", argv[1]);
            break;
        }

        int write_length = fwrite(buffer, sizeof(char), length, fp);

        printf("file_block_length = %d\n", length);
        sum += write_length;
        printf("write_length: %d / %ld\n", sum, Packlength);
        if (sum == Packlength)
            break;

        if (write_length < length)
        {
            printf("File:\t%s Write Failed!\n", file_name);
            break;
        }

        bzero(buffer, BUFFER_SIZE);
    }

    printf(KGRN "Recieve File:\t %s From Server[%s] Finished!\n" RESET, file_name, argv[1]);
    fclose(fp);

    char receiveMessage2[100] = {};
    recv(client_socket, receiveMessage2, sizeof(receiveMessage2), 0);
    printf("Packlength : %s\n", receiveMessage2);

    // 傳輸完畢,關閉socket

    close(client_socket);
    return 0;
}