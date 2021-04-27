/*****************************************************************************
 * updater_device.c    "device Process"
 ***************************************************************************** 
 * second broadcast when receive PC broadcast
 * Receive head {TAG、TotalLength、XactType、HdrSize、Flags、Payloadlength、CRC}
 * Receive packet 、calculate current packet 、rest packet and write each packet 
 * Check CRC and send CRC result
 * Write progress 、read Program process and send
 * Finish
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h> //inet_ntoa
#include <sys/types.h> /* open socket */
#include <unistd.h>	   /* open */
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
//#include <linux/autoconf.h>

#include "mnspdef.h"   //PWIFIDONGLEROMHDR
#include "t6bulkdef.h" //PT6BULKDMAHDR
#include "queue.h"
#include "t6usbdongle.h"
#include "displayserver.h"
#include "jwr2100_tcp_cmddef.h"
#include "CRC.h"

#define KGRN "\033[0;32;32m"
#define RESET "\033[0m"
#define FILE_NAME "UVC-j5-RX-0.0.0.1.bin" //254
#define PACKET_LENGTH 32 * 1024
#define DEVICE_NAME "TEST_NAME"
#define MY_SERVER_IP "192.168.1.121"
#define PORT 55551
#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_SEND 55550
#define HEAD_SIZE 32

int ret, sd, brdcFd, i, optval, calculate_CRC, receiveCRC, clientSocket, flag;
unsigned int tcptotalget, tcptotallength;
unsigned char *rom_buf;
unsigned char buf[64] = {0};
FILE *fp;
PJUVCHDR t6_head;
PDEVICEINFO t6_info;
PJUVCHDR pJUVHdr;
struct sockaddr_in server;
struct sockaddr_in form;
struct timeval tv;

void printmessage(PJUVCHDR t6_head, PDEVICEINFO t6_info) //print head message
{
	if (t6_head != NULL)
	{
		printf("t6_head->Tag = %u\n", t6_head->Tag);
		printf("t6_head->XactType = %d\n", t6_head->XactType);
		printf("t6_head->Flags = %u\n", t6_head->Role);
		printf("t6_head->XactId = %u\n", t6_head->XactId);
		printf("t6_head->TotalLength = %u\n", t6_head->TotalLength);
		printf("t6_head->HdrSize = %d\n", t6_head->HdrSize);
	}
	if (t6_info != NULL)
	{
		printf("t6_info->crc32 = %u\n", t6_info->crc32);
		printf("t6_info->name = %s\n", t6_info->name);
		printf("t6_info->port = %d\n", t6_info->port);
	}
	printf("=========================================================\n");
}
void Broadcast()
{
	//=================================Broadcast Create============================================
	int flag;
	char recvbuf[HEAD_SIZE] = {0};

	brdcFd = socket(PF_INET, SOCK_DGRAM, 0);
	i = 0;
	while (brdcFd < 0 && i < 5) //fail mechanism
	{
		DEBUG_PRINT("Socket Fail\n");
		brdcFd = socket(PF_INET, SOCK_DGRAM, 0);
		i++;
		if (i == 4)
		{
			printf("Please Retry!............\n");
			exit(1);
		}
	}

	//ret = setsockopt(brdcFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	flag = 1;
	optval = 1;
	ret = setsockopt(brdcFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
	ret = setsockopt(brdcFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	ret = setsockopt(brdcFd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));

	bzero(&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = 0x00; //INADDR_ANY = 0
	//inet_addr(BROADCAST_IP);
	server.sin_port = htons(BROADCAST_SEND);

	form.sin_family = AF_INET;
	form.sin_addr.s_addr = htonl(INADDR_BROADCAST); //255.255.255.255
	form.sin_port = htons(55550);

	if ((ret = bind(brdcFd, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) < 0)
	{
		DEBUG_PRINT("bind Fail = %d\n", ret);
		printf("Please Retry!............\n");
		close(brdcFd);
		exit(1);
	}
}
void TCP_Connection()
{
	//=================================TCP Create======================================
	sd = socket(AF_INET, SOCK_STREAM, 0);
	DEBUG_PRINT("test cursor socket sd = %d ip = %s port = %d \n", sd, MY_SERVER_IP, PORT);
	flag = 1;
	ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	i = 0;
	while (ret < 0 && i < 5)
	{
		DEBUG_PRINT("TCP Bind error!\n");
		ret = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
		sleep(1);
		i++;
		if (i == 4)
		{
			printf("Please Retry !............\n");
			close(sd);
			close(brdcFd);
			exit(1);
		}
	}
	DEBUG_PRINT("test cursor Bind successfully.\n");
	ret = listen(sd, 32);
	i = 0;
	while (ret < 0 && i < 5)
	{
		DEBUG_PRINT("Server-listen() Error");
		ret = listen(sd, 32);
		sleep(1);
		i++;
		if (i == 4)
		{
			printf("Please Retry !............\n");
			close(sd);
			close(brdcFd);
			exit(1);
		}
	}

	printf("Server-Ready for client connection...\n");
}
int Receive_data()
{
	int ret = 0, done_len = 0;
	unsigned int write_length, percentage, ;
	rom_buf = malloc(PACKET_LENGTH);
	/*=====Receive packet 、calculate current packet 、rest packet and write each packet=====*/
	tcptotalget = 0;
	percentage = 0;
	while (tcptotallength)
	{
		bzero(rom_buf, PACKET_LENGTH);
		printf("tcptotallength REST =  %d\n", tcptotallength);
		ret = recv(clientSocket, rom_buf, PACKET_LENGTH, 0);
		if (ret <= 0) //disable = 0 ERROR = -1
		{
			DEBUG_PRINT("Recvfrom  Error = %d \n", ret);
			printf("Re-receive Broadcast !..........\n");
			break;
		}

		write_length = fwrite(rom_buf, 1, ret, fp);
		if (write_length < ret)
		{
			DEBUG_PRINT("File:\t%s Write Failed!\n", FILE_NAME);
			printf("Re-receive Broadcast !..........\n");
			break;
		}
		done_len += write_length;
		//tcptotallength = tcptotallength - ret;
		percentage = (done_len / percentage) * 100;
		printf("percentage = \n")
			printf("tcptotalget = %d\n", tcptotalget);
		printf("tcptotallength REST =  %d\n", tcptotallength);
		bzero(buf, sizeof(buf));
		pJUVHdr = (PJUVCHDR)buf;
		pJUVHdr->Tag = 1247106627;
		pJUVHdr->XactType = 3; //XactType = 3 (status)
		pJUVHdr->HdrSize = 32;
		pJUVHdr->TotalLength = 64;
		memcpy(buf + 32, &percentage, sizeof(percentage));
		send(clientSocket, buf, 64, 0);
	}
	if (write_length < ret || ret <= 0)
	{
		free(rom_buf);
		fclose(fp);
		close(clientSocket);
		close(sd);
		close(brdcFd);
		return -1;
	}

	fclose(fp);
	printf(KGRN "File:\t%s Receive Finished!", FILE_NAME);
	printf("\n" RESET);
	return 0;
}
int CRC_Checksum()
{
	char *file_buffer;
	image_header_t head;

	//====================Check CRC and send CRC result==========================
	file_buffer = (char *)malloc(tcptotalget);

	fp = fopen(FILE_NAME, "r");
	if (fp == NULL)
		printf("open file error !......\n");
	printf("file_size = %d\n", tcptotalget);
	bzero(file_buffer, tcptotalget);
	fread(file_buffer, 1, tcptotalget, fp);
	t6_info->crc32 = crc32(0, (const unsigned char *)file_buffer + sizeof(image_header_t), tcptotalget - sizeof(image_header_t)); //32 file crc
	calculate_CRC = t6_info->crc32;

	fp = fopen(FILE_NAME, "r");
	if (fp == NULL)
		printf("open file error !......\n");
	fread(&head, 1, sizeof(image_header_t), fp); //read data CRC
	receiveCRC = ntohl(head.ih_dcrc);

	bzero(buf, sizeof(buf));
	pJUVHdr = (PJUVCHDR)buf;
	pJUVHdr->Tag = 1247106627;
	pJUVHdr->XactType = 4; //XactType = 4 (CRC)
	pJUVHdr->HdrSize = 32;
	pJUVHdr->TotalLength = 64;
	fclose(fp);
	char str[20] = {};
	char str2[15] = {};
	if (calculate_CRC != receiveCRC)
	{
		DEBUG_PRINT("Accept File Content is Error !.........\n");
		printf("calculation CRC32 = %u\n", calculate_CRC);
		printf("Recive t6_head->DeviceInfo.crc32 = %u\n", receiveCRC);
		strcat(str, "CRC = ");
		sprintf(str2, "%u", calculate_CRC);
		strncat(str, str2, strlen(str2));
		memcpy(buf + 32, str, strlen(str));
		send(clientSocket, buf, sizeof(buf), 0);
		//send(clientSocket, &head, sizeof(image_header_t), 0);
		printf("%s\n", str);
		close(clientSocket);
		close(sd);
		close(brdcFd);
		return -1;
	}
	else
	{

		printf("calculation CRC32 = %u\n", calculate_CRC);
		printf("Recive t6_head->DeviceInfo.crc32 = %u\n", receiveCRC);
		printf("Accept Content is Complete !.........\n");
		memcpy(buf + 32, "CRC Complete", 13);
		send(clientSocket, buf, sizeof(buf), 0);
	}
	sleep(2);
	free(file_buffer);
	return 0;
}
int Write_progress()
{
	/*============Write progress 、read Program process and send=================*/
	int progress;
	unsigned int status;
	unsigned char writebuf[4] = {0};
	unsigned char cmd[512] = {0};
	FILE *fp_progress;
	//	bzero(rom_buf, sizeof(rom_buf));
	if (calculate_CRC == receiveCRC) //FW_DEST_RX = 1  rom_head->FW_for_dest == FW_DEST_RX
	{
		int fpeng = open("/home/ryan/samba/updater/RusbFW", O_RDWR | O_CREAT);
		write(fpeng, rom_buf, tcptotalget); //&rom_buf[32 + 512]
		printf("rom_head->TotalLength:%d\n", tcptotalget);
		snprintf(cmd, sizeof(cmd), "/home/ryan/samba/updater/mtd_write -o %d -l %d write %s Kernel &", 0, tcptotalget, "/home/ryan/samba/updater/RusbFW");
		status = system(cmd);
		progress = 0;
		while (progress < 100)
		{
			fp_progress = fopen("/home/ryan/samba/updater/write_file.txt", "r");
			bzero(writebuf, sizeof(writebuf));
			fread(writebuf, 1, sizeof(writebuf), fp_progress);
			sscanf(writebuf, "%d", &progress);
			printf("Font progress : %d\n", progress);
			bzero(buf, sizeof(buf));
			pJUVHdr = (PJUVCHDR)buf;
			pJUVHdr->Tag = 1247106627;
			pJUVHdr->XactType = 3; //XactType = 3 (status)
			pJUVHdr->HdrSize = 32;
			pJUVHdr->TotalLength = 64;
			memcpy(buf + 32, &progress, sizeof(progress));
			send(clientSocket, buf, 64, 0);
			fclose(fp_progress);
			usleep(200000);
		}
		close(clientSocket);
		system("reboot");
	}

	return 0;
}
int main(int argc, char *avg[])
{
	int recvbytes, sendBytes, fromlen;
	unsigned int sin_size, write_length, maxfd;
	unsigned char recvbuf[32] = {0};
	PWIFIDONGLEROMHDR rom_head;
	struct sockaddr_in their_addr;

	Broadcast();
	TCP_Connection();
	fp = fopen(FILE_NAME, "w");
	if (fp < 0)
		printf("file open fail !....\n");
	printf("Waiting Receive BroadCast !.......  ret = %d\n", ret);

	while (1)
	{
		fd_set read_sd;
		FD_ZERO(&read_sd);
		FD_SET(sd, &read_sd);
		FD_SET(brdcFd, &read_sd);
		tv.tv_sec = 6;
		tv.tv_usec = 0;

		maxfd = (sd > brdcFd) ? (sd + 1) : (brdcFd + 1); //which max+1
		ret = select(maxfd + 1, &read_sd, 0, 0, &tv);

		if (ret == -1) //fail
			DEBUG_PRINT("Select Error Re-receive Broadcast !...... ret = %d\n", ret);
		else if (ret == 0) //timeout
			DEBUG_PRINT("Waiting Receive BroadCast !.......  ret = %d\n", ret);

		else
		{ /*TCP*/
			if (FD_ISSET(sd, &read_sd))
			{
				sin_size = sizeof(struct sockaddr_in);
				clientSocket = accept(sd, (struct sockaddr *)&their_addr, &sin_size);
				i = 0;
				if (clientSocket < 0)
				{
					DEBUG_PRINT("Server-accept() Error Re-receive Broadcast !......\n");
					fclose(fp);
					close(clientSocket);
					close(sd);
					close(brdcFd);
					break;
				}
				printf("Server-accept() is OK\n");
				printf("Server-new socket, clientSocket is OK...\n");
				printf("Got connection from the client: %s\n", inet_ntoa(their_addr.sin_addr)); /*client IP*/

				//==============================Receive head===================================
				tcptotallength = 0;
				bzero(buf, sizeof(buf));
				ret = recv(clientSocket, buf, 32, 0);
				if (ret <= 0) //  other side disconnected
				{
					DEBUG_PRINT("Receive head Error = %d \n", ret);
					printf("Re-receive Broadcast !...........\n");
					fclose(fp);
					close(clientSocket);
					close(sd);
					close(brdcFd);
					break;
				}
				t6_head = (PJUVCHDR)buf;
				DEBUG_PRINT("rom recvfrom length ret = %d \n", ret);
				printmessage(t6_head, NULL);

				tcptotallength = t6_head->TotalLength - 32;
				if (t6_head->Tag != 1247106627) //JUVC
				{
					DEBUG_PRINT("Tag Error %d\n", t6_head->Tag);
					printf("Re-receive Broadcast and check Tag !...........\n");
					fclose(fp);
					close(clientSocket);
					close(sd);
					close(brdcFd);
					break;
				}
				//=======================Receive、Checksum、Write===========================
				if (Receive_data() == -1)
					break;
				//* CRC Checksum *//
				if (CRC_Checksum() == -1)
					break;
				if (Write_progress() == -1)
					break;

				free(rom_buf);
				close(clientSocket);
				close(sd);
				close(brdcFd);
				printf(KGRN "Update Complete!");
				printf("\n" RESET);
				return;
			}
			/*Broadcast*/
			else if (FD_ISSET(brdcFd, &read_sd))
			{
				fromlen = sizeof(struct sockaddr_in);
				recvfrom(brdcFd, recvbuf, 32, 0, (struct sockaddr *)&server, &fromlen);
				t6_head = (PJUVCHDR)recvbuf;
				if (t6_head->XactType == JUVC_TYPE_DISCOVER) //JUVC_TYPE_DISCOVER =5
				{
					printf("ret = %d\n", ret);
					bzero(buf, sizeof(buf));
					t6_head = (PJUVCHDR)buf;
					t6_info = (PDEVICEINFO)(buf + 32);
					t6_head->Tag = 1247106627;
					t6_head->XactType = 1;
					t6_head->Role = 1;
					t6_head->XactId = 204;
					t6_head->HdrSize = 32;
					t6_info->port = 55551;
					strcpy(t6_info->name, DEVICE_NAME);

					t6_head = (PJUVCHDR)buf;
					printf("Waiting Receive BroadCast !.............\n");
					sendBytes = sendto(brdcFd, buf, sizeof(struct deviceinfo) + sizeof(struct juvc_hdr_packet), 0, (struct sockaddr *)&form, sizeof(struct sockaddr_in));
					printmessage(t6_head, t6_info);
					//usleep(500000);
				}
			}
		}
	}
}
