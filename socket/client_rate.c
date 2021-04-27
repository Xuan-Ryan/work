//傳輸檔案 顯示進度
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 6667
#define BUFFER_SIZE 1024
#define MAX_SIZE_OF_FILE_NAME 512
#define FILE_NAME "tiantiande.mp3"
#define DEBUG_LOG 0

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

void parse_xml(char *back_str, const char *xml_str, const char *pre_str, const char *suf_str)
{
    if (DEBUG_LOG)
        printf("####L(%d) xml_str:%s pre_str:%s suf_str:%s \n", __LINE__, xml_str, pre_str, suf_str);
    int offset = strlen(pre_str);
    char *ptr_first = strstr(xml_str, pre_str);
    char *ptr_end = strstr(xml_str, suf_str);

    if ((ptr_first != NULL) && (ptr_end != NULL))
    {
        ptr_first = ptr_first + offset;
        strncpy(back_str, ptr_first, (ptr_end - ptr_first));
        if (DEBUG_LOG)
            printf("####L(%d) xml data back_str:%s\n", __LINE__, back_str);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("####L(%d) usage: ./%s serverIpAddress\n", __LINE__, argv[0]);
        exit(1);
    }

    char file_name[16] = {0}; //文件名称
    long file_size = 0;       //文件总大小
    long recv_sum = 0;        //统计接收文件总大小
    int progress = 50;        //进度条的长度
    int current = 0;          //当前进度
    int length, i;

    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0); //0:自动分配一个空闲端口

    int client_socket = socket(AF_INET, SOCK_STREAM, 0); //创建TCP socket套接字
    if (client_socket < 0)
    {
        printf("####L(%d) create socket failed!\n", __LINE__);
        exit(1);
    }

    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr))) //绑定
    {
        printf("####L(%d) client bind port failed!\n", __LINE__);
        exit(1);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (0 == inet_aton(argv[1], &server_addr.sin_addr)) //服务器的IP地址来自程序的参数
    {
        printf("####L(%d) Server IP Address Error!\n", __LINE__);
        exit(1);
    }
    server_addr.sin_port = htons(SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, server_addr_length) < 0)
    {
        printf("####L(%d) Can Not Connect To %s!\n", __LINE__, argv[1]);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    length = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (length < 0)
    {
        printf("####L(%d) server recieve data failed!\n", __LINE__);
        exit(1);
    }

    /*关键代码*/
    struct TCP_STRUCT_DATA struct_data;
    memset(&struct_data, 0x0, sizeof(struct_data));
    memcpy(&struct_data, buffer, sizeof(buffer)); //把结构体的数据通过memcpy的方式拷贝到struct_data中
    printf("####L(%d) cmd:%d data_len:%d\n", __LINE__, struct_data.m_cmd, struct_data.m_data_len);

    if (struct_data.m_data_len > 0) //m_data_len>0 则仍有数据要接收
    {
        printf("####L(%d) going to recv data...\n", __LINE__);

        length = recv(client_socket, buffer, struct_data.m_data_len, 0);
        if (length < 0)
        {
            printf("####L(%d) server recieve data failed!\n", __LINE__);
            exit(1);
        }
        printf("####L(%d) data:%s\n", __LINE__, buffer); //打印要接收的数据部分
        //解析xml

        char file_size_tmp[8] = {0};
        parse_xml(file_name, buffer, "<name>", "</name>");
        parse_xml(file_size_tmp, buffer, "<size>", "</size>");
        if (NULL != file_name)
        {
            file_size = atol(file_size_tmp);
            printf("####L(%d) name:%s, size:%ld\n", __LINE__, file_name, file_size);
        }
        else
        {
            printf("####L(%d) parse xml err！\n", __LINE__);
            exit(1);
        }
    }

    if (0 == access(file_name, F_OK))
    {
        /*如果文件已经存在 先删*/
        if (0 != remove(file_name))
        {
            printf("####L(%d) rm file:%s err！\n", __LINE__, file_name);
        }
    }

    FILE *fp = fopen(file_name, "wb");
    if (NULL == fp)
    {
        printf("####L(%d) file:%s can not open to write\n", __LINE__, file_name);
        exit(1);
    }

    //开始接收文件
    printf("####L(%d) now receiving file...\n", __LINE__);
    while ((length = recv(client_socket, buffer, BUFFER_SIZE, 0)))
    {
        if (length < 0)
        {
            printf("####L(%d) current length < 0 !\n", __LINE__);
            fclose(fp);
            exit(1);
        }
        int write_length = fwrite(buffer, sizeof(char), length, fp); //写文件
        if (write_length < length)
        {
            printf("####L(%d) file:%s write failed write_length<length!\n", __LINE__, file_name);
            fclose(fp);
            exit(1);
        }

        bzero(buffer, BUFFER_SIZE);
        recv_sum += length; //累计接收文件大小

        //计算当前进度
        current = recv_sum / (file_size / progress);
        //打印进度条
        printf("\r");
        printf("[");
        for (i = 0; i < progress; i++)
        {
            if (i < current)
                printf("="); //已接收部分
            else
                printf("+"); //未接收部分
        }
        printf("]");
        printf(" %8ld/%ld %6.2f%%", recv_sum, file_size, (float)recv_sum / file_size * 100);
        //fflush(stdout);

        if (recv_sum == file_size)
        {
            //接收完成
            printf("\n####L(%d) recv file finished ...####\n", __LINE__);
            usleep(500 * 1000);
            break;
        }
        else if (recv_sum > file_size)
        {
            printf("\n####L(%d) recv file err recv_sum[%ld] ...####\n", __LINE__, recv_sum);
            break;
        }
        usleep(1);
    }

    fclose(fp);
    //关闭socket
    close(client_socket);
    return 0;
}