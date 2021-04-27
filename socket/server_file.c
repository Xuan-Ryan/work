////////////////////////////////////////////////////////////////////////
// file_server.c -- socket檔案傳輸伺服器端程式
//斷開後再重新建立連結 解決封包沾黏問題
///////////////////////////////////////////////////////////////////////
//接收任何IP address
//port 9000

#include <stdio.h>		// for printf
#include <stdlib.h>		// for exit
#include <string.h>		// for bzero
#include <sys/types.h>	// for socket
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <openssl/md5.h>

#define KGRN "\033[0;32;32m"
#define RESET "\033[0m"

#define HELLO_WORLD_SERVER_PORT 8080
#define LENGTH_OF_LISTEN_QUEUE 20
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
	//char *message;
	char str[5];
	// set socket's address information
	// 設定一個socket地址結構server_addr,代表伺服器internet的地址和埠
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

	// create a stream socket
	// 建立用於internet的流協議(TCP)socket,用server_socket代表伺服器向客戶端提供服務的介面
	int server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		printf("Create Socket Failed!\n");
		exit(1);
	}

	// 把socket和socket地址結構繫結
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Port: %d Failed!\n", HELLO_WORLD_SERVER_PORT);
		exit(1);
	}

	// server_socket用於監聽
	if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
	{
		printf("Server Listen Failed!\n");
		exit(1);
	}

	// 伺服器端一直執行用以持續為客戶端提供服務
	while (1)
	{

		// 定義客戶端的socket地址結構client_addr,當收到來自客戶端的請求後,呼叫accept
		// 接受此請求,同時將client端的地址和埠等資訊寫入client_addr中
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);

		// 接受一個從client端到達server端的連線請求,將客戶端的資訊儲存在client_addr中
		// 如果沒有連線請求,則一直等待直到有連線請求為止,這是accept函式的特性,可以
		// 用select()來實現超時檢測
		// accpet返回一個新的socket,這個socket用來與此次連線到server的client進行通訊
		// 這裡的new_server_socket代表了這個通訊通道
		int new_server_socket = accept(server_socket, (struct sockaddr *)&client_addr, &length);
		if (new_server_socket < 0)
		{
			printf("Server Accept Failed!\n");
			break;
		}

		char buffer[BUFFER_SIZE];
		bzero(buffer, sizeof(buffer));
		length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
		if (length < 0)
		{
			printf("Server Recieve Data Failed!\n");
			break;
		}

		char file_name[FILE_NAME_MAX_SIZE + 1];
		bzero(file_name, sizeof(file_name));
		strncpy(file_name, buffer,
				strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));

		FILE *fp = fopen(file_name, "r");
		if (fp == NULL)
		{
			printf("File:\t%s Not Found!\n", file_name);
		}

		else
		{
			bzero(buffer, BUFFER_SIZE);
			int file_block_length = 0;
			while ((file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
			{
				printf("file_block_length = %d\n", file_block_length);

				sprintf(str, "%d", file_block_length);
				if (send(new_server_socket, str, file_block_length, 0) < 0)
				{
					printf("Send File:\t%s Failed!\n", file_name);
					break;
				}

				printf("Sendpack : %s\n", str);

				usleep(1000 * 1000); //要休眠一下 否则第二次发过去的数据可能来不及接收到

				// 傳送buffer中的字串到new_server_socket,實際上就是傳送給客戶
				if (send(new_server_socket, buffer, file_block_length, 0) < 0)
				{
					printf("Send File:\t%s Failed!\n", file_name);
					break;
				}
				bzero(buffer, sizeof(buffer));
			}

			fclose(fp);

			printf("File:\t%s Transfer Finished!\n", file_name);
			close(new_server_socket);
			//會造成每一行檔案後會有^M   可分成兩組
		}
		//放這會合new_server_socket   server_socket 只一次
		usleep(1000 * 1000); //要休眠一下 否则第二次发过去的数据可能来不及接收到

		new_server_socket = accept(server_socket, (struct sockaddr *)&client_addr, &length);
		if (new_server_socket < 0)
		{
			printf("Server Accept Failed!\n");
			break;
		}

		char message[] = {"Hi,this is server.\n"};
		if (send(new_server_socket, message, sizeof(message), 0) < 0)
		{
			printf("Send File:\t%s Failed!\n", file_name);
			break;
		}

		close(new_server_socket);

		//只收到一次
	}
	close(server_socket);
	return 0;
}
