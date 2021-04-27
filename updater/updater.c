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
#include <unistd.h>	/* open */
#include <sys/stat.h>  /* open */
#include <fcntl.h>	 /* open */
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
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ifaddrs.h>

//#include <linux/autoconf.h>

#include "updater.h"
#include "CRC.h"

#define KGRN "\033[0;32;32m"
#define RESET "\033[0m"
#define FILE_NAME "/var/tmpFW" //   UVC-j5-RX-0.0.0.1.bin
#define PACKET_LENGTH 32 * 1024
#define DEVICE_NAME "TEST_NAME"
#define PORT 55551
#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_SEND 55550
#define HEAD_SIZE 32

int tcp_sd, brdcfd;
unsigned int tcptotalget;
struct sockaddr_in server; //br receform
struct sockaddr_in form;   //br sendto
struct timeval tv;

void Close_socket(int acceptfd, int tcp_sd, int brdcfd)
{
	if (acceptfd > 0)
	{
		close(acceptfd);
		acceptfd = 0;
	}
	if (tcp_sd > 0)
	{
		close(tcp_sd);
		tcp_sd = 0;
	}
	if (brdcfd > 0)
	{
		close(brdcfd);
		brdcfd = 0;
	}
}
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
void Broadcast_create()
{
	//=================================Broadcast Create============================================
	int flag, ret, i;
	char recvbuf[HEAD_SIZE] = {0};
	//struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	brdcfd = socket(PF_INET, SOCK_DGRAM, 0);
	i = 0;
	while (brdcfd < 0 && i < 5) //fail mechanism
	{
		printf("Socket Fail\n");
		brdcfd = socket(PF_INET, SOCK_DGRAM, 0);
		i++;
		if (i == 4)
		{
			printf("Please Retry!............\n");
			exit(1);
		}
	}

	//ret = setsockopt(brdcfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	flag = 1;
	ret = setsockopt(brdcfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
	ret = setsockopt(brdcfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	ret = setsockopt(brdcfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));

	bzero(&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = 0x00; //INADDR_ANY = 0
	server.sin_port = htons(BROADCAST_SEND);

	form.sin_family = AF_INET;
	form.sin_addr.s_addr = htonl(INADDR_BROADCAST); //255.255.255.255
	form.sin_port = htons(55550);
	i = 0;
	while ((ret = bind(brdcfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) < 0 && i < 5)
	{
		printf("bind Fail = %d\n", ret);
		ret = bind(brdcfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
		i++;
		if (i == 4)
		{
			printf("Please Retry!............\n");
			Close_socket(0, brdcfd, tcp_sd);
			exit(1);
		}
	}
}
void TCP_create()
{
	int ret, flag;
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	tcp_sd = socket(AF_INET, SOCK_STREAM, 0);
	flag = 0;
	while (tcp_sd < 0 && flag < 5)
	{
		printf("Socket Create Error!\n");
		tcp_sd = socket(AF_INET, SOCK_STREAM, 0);
		sleep(1);
		if (flag == 4)
		{
			printf("Please Retry!............\n");
			exit(1);
		}
	}
	printf("test cursor socket tcp_sd = %d ip = %s port = %d \n", tcp_sd, "INADDR_ANY", PORT);
	flag = 1;
	ret = setsockopt(tcp_sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
	ret = setsockopt(tcp_sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	struct sockaddr_in tcp_addr;
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(PORT);
	tcp_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr(ip[1]);

	ret = bind(tcp_sd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr));
	flag = 0;
	while (ret < 0 && flag < 5)
	{
		printf("TCP Bind error!\n");
		ret = bind(tcp_sd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr));
		sleep(1);
		flag++;
		if (flag == 4)
		{
			printf("Please Retry !............\n");
			Close_socket(0, brdcfd, tcp_sd);
			exit(1);
		}
	}
	printf("test cursor Bind successfully.\n");
	ret = listen(tcp_sd, 5);
	flag = 0;
	while (ret < 0 && flag < 5)
	{
		printf("Server-listen() Error");
		ret = listen(tcp_sd, 32);
		sleep(1);
		flag++;
		if (flag == 4)
		{
			printf("Please Retry !............\n");
			Close_socket(0, brdcfd, tcp_sd);
			exit(1);
		}
	}

	printf("Server-Ready for client connection...\n");
}
int Receive_data(int sd)
{
	int ret, calculate_file;
	unsigned char buf[64] = {0};
	unsigned char *rom_buf;
	unsigned int write_length, percentage, tcptotallength;
	PJUVCHDR t6_head;
	PJUVCHDR pJUVHdr;
	FILE *fp;

	//==============================Receive head===================================
	fp = fopen(FILE_NAME, "w");
	if (fp == NULL)
	{
		return -1;
	}
	tcptotallength = 0;
	bzero(buf, sizeof(buf));
	ret = recv(sd, buf, 32, 0);
	if (ret <= 0) //  other side disconnected
	{
		printf("Receive head Error = %d \n", ret);
		printf("Re-receive Broadcast !...........\n");
		fclose(fp);
		return -1;
	}
	t6_head = (PJUVCHDR)buf;
	printf("rom recvfrom length ret = %d \n", ret);
	printmessage(t6_head, NULL);
	if (t6_head->Tag != 1247106627) //JUVC
	{
		printf("Tag Error = %d\n", t6_head->Tag);
		printf("Re-receive Broadcast and check Tag !...........\n");
		fclose(fp);
		return -1;
	}
	if (t6_head->XactType == 6) // 6 ready
	{
		t6_head->Role = 1;
		ret = send(sd, t6_head, 32, 0);
	}

	bzero(buf, sizeof(buf));
	printf("Receive head Error = %d \n", ret);
	printf("Re-receive Broadcast !...........\n");
	ret = recv(sd, buf, 32, 0);
	if (ret <= 0) //  other side disconnected
	{
		printf("Receive head Error = %d \n", ret);
		printf("Re-receive Broadcast !...........\n");
		fclose(fp);
		return -1;
	}

	if (t6_head->XactType == JUVC_TYPE_UPDATE_DATA) //need update
	{
		tcptotallength = t6_head->TotalLength - 32;
		if (tcptotallength > (15 * 1024 * 1024) || tcptotallength <= 32) //JUVC
		{
			//printf("File size Error = %d\n", tcptotallength);
			printf("Re-receive Broadcast and check File !...........\n");
			fclose(fp);
			return -1;
		}
		/*=====Receive packet 、calculate current packet 、rest packet and write each packet=====*/

		rom_buf = malloc(PACKET_LENGTH);
		tcptotalget = 0;
		calculate_file = tcptotallength;
		printf("Receive head Error = %d \n", ret);
		printf("Re-receive Broadcast !...........\n");
		while (tcptotallength)
		{
			bzero(rom_buf, PACKET_LENGTH);
			ret = recv(sd, rom_buf, PACKET_LENGTH, 0);
			if (ret <= 0) //disable = 0 ERROR = -1
			{
				printf("Recvfrom  Error = %d \n", ret);
				printf("Re-receive Broadcast !..........\n");
				break;
			}
			write_length = fwrite(rom_buf, 1, ret, fp);
			printf("write_length = %d\n", write_length);
			if (write_length < ret)
			{
				printf("File:\t%s Write Failed!\n", FILE_NAME);
				printf("Re-receive Broadcast !..........\n");
				break;
			}
			printf("rom recvfrom ret = %d \n", ret);
			tcptotalget += ret;
			tcptotallength -= ret;
			percentage = ((float)tcptotalget / calculate_file) * 100;
			printf("percentage = %d\n", percentage);
			printf("tcptotalget =  %d\n", tcptotalget);
			printf("tcptotallength REST =  %d\n", tcptotallength);
			bzero(buf, sizeof(buf));
			pJUVHdr = (PJUVCHDR)buf;
			pJUVHdr->Tag = 1247106627;
			pJUVHdr->XactType = 7; //XactType = 7 (receive status)
			pJUVHdr->HdrSize = 32;
			pJUVHdr->TotalLength = 64;
			memcpy(buf + 32, &percentage, sizeof(percentage));
			send(sd, buf, 64, 0);
			//usleep(50000);
		}
	}
	if (write_length < ret || ret <= 0)
	{
		free(rom_buf);
		fclose(fp);
		return -1;
	}
	free(rom_buf);
	fclose(fp);
	printf(KGRN "File:\t%s Receive Finished!", FILE_NAME);
	printf("\n" RESET);
	return 0;
}
int CRC_checksum(int sd)
{
	int calculate_CRC, receiveCRC;
	char *file_buffer;
	unsigned char buf[64] = {0};
	PDEVICEINFO t6_info;
	PJUVCHDR pJUVHdr;
	FILE *fp;
	image_header_t head;
	//====================Check CRC and send CRC result==========================
	file_buffer = (char *)malloc(tcptotalget);

	fp = fopen(FILE_NAME, "r");
	if (fp == NULL)
		printf("open file error !......\n");
	printf("file_size = %d\n", tcptotalget);
	bzero(file_buffer, tcptotalget);
	fread(file_buffer, 1, tcptotalget, fp);
	t6_info = (PDEVICEINFO)(buf + 32);
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
		printf("Accept File Content is Error !.........\n");
		printf("calculation CRC32 = %u\n", calculate_CRC);
		printf("Recive t6_head->DeviceInfo.crc32 = %u\n", receiveCRC);
		//strcat(str, "CRC = ");
		//sprintf(str2, "%u", calculate_CRC);
		//strncat(str, str2, strlen(str2));
		memcpy(buf + 32, "File Content is Error !", 24);
		send(sd, buf, sizeof(buf), 0);
		printf("%s\n", str);
		free(file_buffer);
		close(sd);
		return -1;
	}
	else
	{
		printf("calculation CRC32 = %u\n", calculate_CRC);
		printf("Recive t6_head->DeviceInfo.crc32 = %u\n", receiveCRC);
		printf("Accept Content is Complete !.........\n");
		memcpy(buf + 32, "Content is Complete", 20);
		send(sd, buf, sizeof(buf), 0);
	}
	sleep(1);
	free(file_buffer);
	return 0;
}
int Write_progress(int sd)
{
	/*============Write progress 、read Program process and send=================*/
	int progress, fpeng;
	unsigned char writebuf[4] = {0};
	unsigned char cmd[512] = {0};
	unsigned char buf[64] = {0};
	PJUVCHDR pJUVHdr;
	FILE *fp_progress;

	printf("File_TotalLength:%d\n", tcptotalget);
	snprintf(cmd, sizeof(cmd), "mtd_write -o %d -l %d write %s Kernel &", 0, tcptotalget, FILE_NAME); //FILE_NAME
	//snprintf(cmd, sizeof(cmd), "/home/ryan/samba/updater/mtd_write -o %d -l %d write %s Kernel &", 0, tcptotalget, "/home/ryan/samba/updater/RusbFW");
	system(cmd);

	system("rm /var/mtd_write.log");
	system("touch /var/mtd_write.log");
	//system("rm write_file.txt");
	//system("touch write_file.txt");
	usleep(10000);

	progress = 0;
	while (progress < 100)
	{
		fp_progress = fopen("/var/mtd_write.log", "r"); // /var/mtd_write.log  /home/ryan/samba/updater/write_file.txt
		if (fp_progress == NULL)
		{
			printf("File open fail, %d\n", errno);
			break;
		}
		bzero(writebuf, sizeof(writebuf));
		fread(writebuf, 1, sizeof(writebuf), fp_progress);
		sscanf(writebuf, "%d", &progress);
		printf("Font progress : %d\n", progress);
		bzero(buf, sizeof(buf));
		pJUVHdr = (PJUVCHDR)buf;
		pJUVHdr->Tag = 1247106627;
		pJUVHdr->XactType = 3; //XactType = 3 (write status)
		pJUVHdr->HdrSize = 32;
		pJUVHdr->TotalLength = 64;
		memcpy(buf + 32, &progress, sizeof(progress));
		send(sd, buf, 64, 0);
		fclose(fp_progress);
		usleep(200000);
	}
	return 0;
}
int main(int argc, char *avg[])
{
	int recvbytes, sendBytes, fromlen, ret, CRCret, acceptfd;
	unsigned int sin_size, write_length, maxfd;
	unsigned char buf[64] = {0};
	PJUVCHDR t6_head;
	PDEVICEINFO t6_info;
	struct timeval tv;
	struct sockaddr_in their_addr;

	Broadcast_create();
	TCP_create();
	ret = 0;
	printf("Waiting Receive BroadCast !.......  ret = %d\n", ret);
	fd_set read_sd;
	FD_ZERO(&read_sd);
	signal(SIGPIPE, SIG_IGN);
	//system("killall apclient_chkconn.sh");

	while (1)
	{
		FD_SET(tcp_sd, &read_sd);
		FD_SET(brdcfd, &read_sd);
		tv.tv_sec = 6;
		tv.tv_usec = 0;
		maxfd = (tcp_sd > brdcfd) ? (tcp_sd + 1) : (brdcfd + 1); //which max+1
		ret = select(maxfd + 1, &read_sd, 0, 0, &tv);

		if (ret == 0) //timeout
			printf("Waiting Receive BroadCast !.......  ret = %d\n", ret);
		else if (ret == -1)
		{ //fail
			printf("Select Error Re-receive Broadcast !...... ret = %d\n", ret);
			Broadcast_create();
			TCP_create();
			FD_ZERO(&read_sd);
			ret = 0;
		}
		// add tcp connect
		else if (FD_ISSET(tcp_sd, &read_sd))
		{
			system("killall apclient_chkconn.sh");
			sin_size = sizeof(struct sockaddr_in);
			acceptfd = accept(tcp_sd, (struct sockaddr *)&their_addr, &sin_size);
			if (acceptfd < 0)
			{
				printf("Server-accept() Error Re-receive Broadcast !......\n");
				Close_socket(acceptfd, tcp_sd, brdcfd);
				acceptfd = 0;
				break;
			}
			printf("Server-accept() is OK\n");
			printf("Server-new socket, acceptfd is OK...\n");
			printf("Got connection from the client: %s\n", inet_ntoa(their_addr.sin_addr)); // *client IP

			FD_SET(acceptfd, &read_sd); // add master set
			if (acceptfd >= maxfd)
			{ // max fd
				maxfd = acceptfd;
			}
		}
		// *Broadcast
		else if (FD_ISSET(brdcfd, &read_sd))
		{
			fromlen = sizeof(struct sockaddr_in);
			bzero(buf, sizeof(buf));
			recvfrom(brdcfd, buf, 32, 0, (struct sockaddr *)&server, &fromlen);
			t6_head = (PJUVCHDR)buf;
			if (t6_head->XactType == JUVC_TYPE_DISCOVER) //JUVC_TYPE_DISCOVER =5
			{
				printf("ret = %d\n", ret);
				bzero(buf, sizeof(buf));
				t6_head = (PJUVCHDR)buf;
				t6_info = (PDEVICEINFO)(buf + 32);
				t6_head->Tag = 1247106627;
				t6_head->XactType = 1;
				t6_head->Role = JUVC_DEVICE_TYPE_TX; //TX or RX select
				t6_head->XactId = 204;
				t6_head->HdrSize = 32;
				t6_info->port = 55551;
				strcpy(t6_info->name, DEVICE_NAME);
				sendBytes = sendto(brdcfd, buf, sizeof(struct deviceinfo) + sizeof(struct juvc_hdr_packet), 0, (struct sockaddr *)&form, sizeof(struct sockaddr_in));
				printmessage(t6_head, t6_info);
			}
		}
		// tcp acceptfd
		else if (FD_ISSET(acceptfd, &read_sd))
		{
			printf("acceptfd = %d\n", acceptfd);
			if ((ret = Receive_data(acceptfd)) == -1)
			{
				Close_socket(acceptfd, tcp_sd, brdcfd);
				printf("Receive_data = %d\n", ret);
				continue;
			}

			// CRC Checksum
			if ((ret = CRC_checksum(acceptfd)) == -1)
			{
				Close_socket(acceptfd, tcp_sd, brdcfd);
				printf("CRC_checksum = %d\n", ret);
				continue;
			}

			if ((ret = Write_progress(acceptfd)) == -1)
			{
				Close_socket(acceptfd, tcp_sd, brdcfd);
				printf("Write_progress = %d\n", ret);
				continue;
			}

			Close_socket(acceptfd, tcp_sd, brdcfd);
			printf(KGRN "Update Complete!");
			printf("\n" RESET);
			system("reboot");
			return;
		}
	}
}
