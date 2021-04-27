//8M更新檔
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h> //inet_ntoa
#include <sys/types.h> /* open socket */
#include <unistd.h>
#include <sys/stat.h>  /* open */
#include <fcntl.h>	   /* open */
#include <sys/ioctl.h> /* ioctl */
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include <stdbool.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include <errno.h>
#include <linux/autoconf.h>

#include "mnspdef.h"   //PWIFIDONGLEROMHDR
#include "t6bulkdef.h" //PT6BULKDMAHDR
#include "queue.h"
#include "t6usbdongle.h"
#include "displayserver.h"
#include "jwr2100_tcp_cmddef.h"

unsigned char MT7620_update_flag = 0;
unsigned char MT7620_tcp_accept_flag = 0;
char *my_server_ip_addr = "10.10.10.253";

void *RomUpdateTcp()
{
	int ret, i;
	unsigned char buf[20 * 1024];
	unsigned char *rom_buf;
	struct sockaddr_in from;
	struct timeval tv;
	socklen_t len;
	char *ptr_image_pice;
	char *ptr_image;
	char *image_data = NULL;

	int packet_frist = -1;
	int packet_id = -1;
	int packet_len = -1;
	int ipAddr = 0;
	int image_size = -1;
	struct sockaddr_in their_addr;
	int clientSocket = -1;
	int sin_size = sizeof(struct sockaddr_in);

	len = sizeof(from);

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		//------------- #define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args) in dispalyserver
		DEBUG_PRINT("socket create error!\n"); //將信息列印到文檔中
		return;
	}
	DEBUG_PRINT("test cursor socket fd=%d ip = %s port=%d \n", fd, my_server_ip_addr, JWR_ROM_TCP_PORT);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(JWR_ROM_TCP_PORT); //-----------
	addr.sin_addr.s_addr = inet_addr(my_server_ip_addr);

	/*
	tv.tv_sec =  1;
	tv.tv_usec = 0;
	if ( setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval)) == -1) {
		DEBUG_PRINT("setsockopt SO_RCVTIMEO  error\n");
		close(fd);
		return ;
	}
*/
	//-----
	int n = TCP_MAX_BUFFER;											//TCP_MAX_BUFFER = 8388608 in displayserver.h
	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) == -1) //設定套介面的選項  設置接收緩衝區   SOL_SOCKET:通用通訊端選項    SO_RCVBUF 接收緩衝區大小(int)
	{
		DEBUG_PRINT("setsockopt SO_RCVBUF  error"); //-----------------
		close(fd);
		return;
	}

	ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1)
	{
		DEBUG_PRINT("test cursor Bind error!\n");
		close(fd);
		return;
	}
	DEBUG_PRINT("test cursor Bind successfully.\n");

	if ((ret = listen(fd, 32)) < 0)
	{
		DEBUG_PRINT("Server-listen() error");
		close(fd);
		fd = -1;
		return -4;
	}
	else
	{
		printf("Server-Ready for client connection...\n");
	}

	while (1)
	{
		if ((clientSocket = accept(fd, (struct sockaddr *)&their_addr, &sin_size)) < 0)
		{
			printf("Server-accept() error \n");
			close(fd);
			fd = -1;
			return -2;
		}
		else
		{
			printf("Server-accept() is OK\n");
			/*client IP*/
			printf("Server-new socket, clientSocket is OK...\n");
			printf("Got connection from the client: %s\n", inet_ntoa(their_addr.sin_addr)); //將網絡地址轉換成“.”點隔的字符串格式
			//printf("Got connection from the client: %d\n", *paddr);
			MT7620_tcp_accept_flag = 1;
			FILE *fpeng001 = open("/tmp/MT7620_tcp_accept_flag", O_RDWR | O_CREAT);
			write(fpeng001, &MT7620_tcp_accept_flag, 1); //要寫入的資料,長度
			close(fpeng001);
		}
		//---------
		fd_set read_sd;					//是一组文件描述字(fd)的集合 使用於select
		FD_ZERO(&read_sd);				//清空集合
		FD_SET(clientSocket, &read_sd); //将一个给定的文件描述符加入集合之中

		while (clientSocket)
		{
			fd_set rsd = read_sd;
			int sel = select(clientSocket + 1, &rsd, 0, 0, 0); //select 使socket非堵塞  测试指定的fd可读？可写？有异常条件待处理？     clientSocket + 1(是一个整数值，是指集合中所有文件描述符的范围，即所有文件描述符的最大值加1)

			if (sel > 0)
			{
				//ipAddr = g_active_display_ip;
				PT6BULKDMAHDR t6_head = (PT6BULKDMAHDR)buf; //---------------------------
				unsigned int tcptotalget = 0;
				unsigned int tcptotallength = 0;
				unsigned int align64k_len = 0;

				ret = recv(clientSocket, &buf[tcptotalget], 32, 0); //client recive  &buf[tcptotalget] = buf
				if (ret <= 0)
				{
					//DEBUG_PRINT("cusor recvfrom  error = %d \n",ret);
					continue;
				}

				DEBUG_PRINT("rom recvfrom ret = %d \n", ret);

				if (t6_head->Signature != 0x05) //-------------
				{
					DEBUG_PRINT("Signature error\n");
					//Dump_MnspXactHdr(t6_head);
					continue;
				}

				rom_buf = malloc(t6_head->PayloadLength + 32);
				memcpy(rom_buf, &buf[tcptotalget], 32); //內存拷貝函數 從源*str2所指的內存地址的起始位置開始拷貝n個字節到目標*str1所指的內存地址的起始位置中。
				tcptotallength = t6_head->PayloadLength;
				tcptotalget = tcptotalget + 32;											//
				PWIFIDONGLEROMHDR rom_head = (PWIFIDONGLEROMHDR)&rom_buf[32 + 0x40000]; //設定結構起始位置---------------0x40000
				MT7620_update_flag = 1;
				FILE *fpeng000 = open("/tmp/MT7620_update_flag", O_RDWR | O_CREAT);
				write(fpeng000, &MT7620_update_flag, 1);
				close(fpeng000);

				while (tcptotallength) //接收封包扣完跳出
				{
					ret = recv(clientSocket, &rom_buf[tcptotalget], tcptotallength, 0);
					if (ret <= 0) //錯誤 recv=-1   //對方關閉recv=0
					{
						//DEBUG_PRINT("cusor recvfrom  error = %d \n",ret);
						continue;
					}
					DEBUG_PRINT("rom recvfrom ret = %d \n", ret);
					tcptotalget = tcptotalget + ret; //總共得到的封包大小
					tcptotallength = tcptotallength - ret;
				}

				if (rom_head->FW_for_dest == FW_DEST_RX) //-------------------  FW_DEST_RX=1
				{
					//fpeng = open("/home/ryan/samba/updater/RusbFW", O_RDWR | O_CREAT);
					//int fp = fopen(FILE_NAME, "r");
					//write(fpeng, file_buffer, tcptotalget);
					//write(fpeng, rom_buf, tcptotalget); //&rom_buf[32 + 512]
					FILE *fpeng = open("/var/RusbFW", O_RDWR | O_CREAT);
					char cmd[512];
					int status;
					write(fpeng, &rom_buf[32 + 512 + 0x40000], rom_head->PayloadLength); //write buf>feng  &rom_buf[32 + 512 + 0x40000] 跳過bootload
					printf("rom_head->PayloadLength:%d\n", rom_head->PayloadLength);
					snprintf(cmd, sizeof(cmd), "/bin/mtd_write -o %d -l %d write %s Kernel", 0, rom_head->PayloadLength, "/var/RusbFW"); //放置/bin/. 才可在電腦跑
					status = system(cmd);																								 //可執行linux指令
					send(clientSocket, &MT7620_update_flag, 1, 0);
					close(clientSocket);
					system("reboot");
				}
			}
			else if (sel < 0)
			{
				break;
			}
		}

		close(clientSocket);
	}

	if (image_data != NULL) //------------
	{
		free(image_data);
		image_data = NULL;
	}
	close(fd);
}

int main(int argc, char *avg[])
{
	pthread_t pthr_cursor_tcp; //---------------

	FILE *fpeng001 = open("/tmp/MT7620_tcp_accept_flag", O_RDWR | O_CREAT); //O_RDWR 可读可写打开   O_CREAT 若此文件不存在则创建它。使用此选项时需要提供第三个参数mode，表示该文件的访问权限。
	write(fpeng001, &MT7620_tcp_accept_flag, 1);
	close(fpeng001);

	FILE *fpeng000 = open("/tmp/MT7620_update_flag", O_RDWR | O_CREAT);
	write(fpeng000, &MT7620_update_flag, 1);
	close(fpeng000);

	RomUpdateTcp();
}

/*
		DEBUG_PRINT("Accept File Content is Error !.........\n");
		printf("calculation CRC32 = %u\n", calculate_CRC);
		printf("Recive t6_head->DeviceInfo.crc32 = %u\n", receiveCRC);
		//strcat(str, "CRC = ");
		//sprintf(str2, "%u", calculate_CRC);
		//strncat(str, str2, strlen(str2));
		//memcpy(buf + 32, str, strlen(str));
		memcpy(buf + 32, "NG", 2);
		send(accept_socket, buf, sizeof(buf), 0);
		//send(accept_socket, &head, sizeof(image_header_t), 0);
		printf("%s\n", str);
		free(file_buffer);
		close(accept_socket);
		return -1;
		*/