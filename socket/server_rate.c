//傳輸檔案 顯示進度
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //close()
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <assert.h>

#define SERVER_PORT 8080
#define MAX_CONNETCION_COUNT 20 //最大连接数
#define BUFFER_SIZE 1024
#define MAX_SIZE_OF_FILE_NAME 512
#define FILE_NAME "tiantiande.mp3"

struct TCP_STRUCT_DATA
{
    int m_cmd;
    int m_data_len;
};

enum TCP_CMD
{
    GET_FILE = 0,
    //...
};

long get_file_size(char *file_name)
{
    assert(file_name != NULL);
    if (access(file_name, F_OK))
    {
        printf("####L(%d) file:%s not exist!", __LINE__, file_name);
        return -1;
    }

    FILE *fp = fopen(file_name, "rb");
    if (NULL == fp)
    {
        printf("file:%s can not open!\n", file_name);
        return -1;
    }

    long file_size = 0;
    fseek(fp, 0, SEEK_END); //把文件内部指针移动到文件尾部
    file_size = ftell(fp);  //返回指针偏离文件头的位置(即文件中字符个数)
    fseek(fp, 0, SEEK_SET); //把文件内部指针移回到文件头部

    fclose(fp);

    return file_size;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY); //允许任何IP连接server
    //server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    int server_socket = socket(PF_INET, SOCK_STREAM, 0); //创建TCP socket套接字
    if (server_socket < 0)
    {
        printf("####L(%d) create socket failed!", __LINE__);
        exit(1);
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) //绑定端口
    {
        printf("####L(%d) bind port : %d failed!", __LINE__, SERVER_PORT);
        exit(1);
    }

    if (listen(server_socket, MAX_CONNETCION_COUNT)) //监听
    {
        printf("####L(%d) server listen failed!", __LINE__);
        exit(1);
    }

    while (1) //服务器端一直运行
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);

        int new_server_socket = accept(server_socket, (struct sockaddr *)&client_addr, &length);
        if (new_server_socket < 0)
        {
            printf("####L(%d) server accept failed!\n", __LINE__);
            break;
        }

        char data_tmp[512] = {0}; //要发的数据部分内容
        long file_size = get_file_size(FILE_NAME);
        if (-1 == file_size)
        {
            printf("####L(%d) get_file_size err!\n", __LINE__);
            break;
        }
        printf("%ld", file_size);

        sprintf(data_tmp, "<file><name>%s</name><size>%ld</size></file>", FILE_NAME, file_size); //要发送的数据部分内容

        struct TCP_STRUCT_DATA struct_data; //数据头
        memset(&struct_data, 0x0, sizeof(struct_data));
        struct_data.m_cmd = GET_FILE;              //获取文件命令
        struct_data.m_data_len = strlen(data_tmp); //数据部分真实的长度
        //struct_data.m_data_len =sizeof(data_tmp);//512是错误的

        /*关键部分*/
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        int send_len = sizeof(struct_data);
        memcpy(buffer, &struct_data, send_len);

        //向服务器发送buffer中的数据
        int len = send(new_server_socket, buffer, send_len, 0); //发数据头部分
        if (len < 0)
        {
            printf("####L(%d) send err...\n", __LINE__); //发送失败
            break;
        }

        usleep(100 * 1000); //要休眠一下 否则第二次发过去的数据可能来不及接收到
        len = send(new_server_socket, data_tmp, struct_data.m_data_len, 0);
        if (len < 0)
        {
            printf("####L(%d) send err...\n", __LINE__); //发送失败
            break;
        }

        usleep(100 * 1000); //要休眠一下 否则第二次发过去的数据可能来不及接收到

        FILE *fp = fopen(FILE_NAME, "rb"); //以二進制打開

        if (NULL == fp)
        {
            printf("####L(%d) file:%s not found!\n", __LINE__, FILE_NAME);
        }
        else
        {
            int file_block_length = 0;
            bzero(buffer, BUFFER_SIZE);                                                    //缓冲区清0
            while ((file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) //读文件
            {
                printf("####L(%d) file_block_length = %d\n", __LINE__, file_block_length);
                if (send(new_server_socket, buffer, file_block_length, 0) < 0) //发送文件数据
                {
                    printf("####L(%d) send file:%s failed\n", __LINE__, FILE_NAME);
                    break;
                }
                bzero(buffer, BUFFER_SIZE);
            }
            fclose(fp); //決定傳輸檔案關閉
            printf("####L(%d) file:%s transfer finished!\n", __LINE__, FILE_NAME);
        }
        //关闭与客户端的连接
        close(new_server_socket);
    }
    //关闭监听用的socket
    close(server_socket);
    return 0;
}
