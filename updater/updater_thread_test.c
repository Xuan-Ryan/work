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
#include <arpa/inet.h> //inet_ntoa
#include <sys/types.h> /* open socket */
#include <unistd.h>	   /* open */
#include <sys/stat.h>  /* open */
#include <fcntl.h>	   /* open */
#include <sys/ioctl.h> /* ioctl */
#include <signal.h>	   /* SIGPIPE */
#include <errno.h>
#include <pthread.h>
#include <net/if.h>

#include "updater.h"
#include "CRC.h"

#define KGRN "\033[0;32;32m"
#define RESET "\033[0m"
#define FILE_NAME "/var/tmpFW" //   UVC-j5-RX-0.0.0.1.bin
#define PACKET_LENGTH_MAX 32 * 1024
#define PORT 55551
#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_PORT 55550

int tcp_sd, broadcastsd, progress;
unsigned int tcptotalget;
struct sockaddr_in server; //br receform
struct sockaddr_in form;   //br sendto

char *get_macaddr(char *ifname)
{
	struct ifreq ifr;
	//char *ptr;
	int skfd;
	static char if_hw[7] = {0};
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "%s: open socket error\n", __func__);
		return NULL;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);

	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	{
		close(skfd);
		fprintf(stderr, "%s: ioctl fail\n", __func__);
		return NULL;
	}
	//ptr = (char *)&ifr.ifr_addr.sa_data;
	int i = 0;
	while (i < 6)
	{
		if_hw[i] = (long int)ifr.ifr_addr.sa_data[i] & 0xff;
		printf("%02X\n", if_hw[i]);
		i++;
	}
	if_hw[i] = '\0';
	/*
	sprintf(if_hw, "%02X:%02X:%02X:%02X:%02X:%02X",
			(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
			(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));
			*/

	close(skfd);
	return if_hw;
}
void close_socket(int acceptfd, int tcp_sd, int broadcastsd)
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
	if (broadcastsd > 0)
	{
		close(broadcastsd);
		broadcastsd = 0;
	}
}
void printmessage(PJUVCHDR t6_head, PDEVICEINFO t6_info) //print head message
{
	printf("\n thread !!!!!!!!!!!!!!!!!\n");
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
		printf("t6_info->DeviceName = %s\n", t6_info->DeviceName);
		printf("t6_info->Port = %d\n", t6_info->Port);
		printf("t6_info->Version = %s\n", t6_info->Version);
		printf("t6_info->ModelName = %s\n", t6_info->ModelName);
		printf("t6_info->MacAddress = %02X", t6_info->MacAddress[0]);
		printf(":%02X", t6_info->MacAddress[1]);
		printf(":%02X", t6_info->MacAddress[2]);
		printf(":%02X", t6_info->MacAddress[3]);
		printf(":%02X", t6_info->MacAddress[4]);
		printf(":%02X\n", t6_info->MacAddress[5]);
	}
	printf("=========================================================\n");
}
void broadcast_create()
{
	//=================================Broadcast Create============================================
	int flag, ret;
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	do
	{
		broadcastsd = socket(PF_INET, SOCK_DGRAM, 0);
		if (broadcastsd > 0)
			break;
		printf("Broadcast Socket Fail... %d\n", broadcastsd);
		sleep(1);
	} while (1);

	//ret = setsockopt(broadcastsd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	flag = 1;
	ret = setsockopt(broadcastsd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
	ret = setsockopt(broadcastsd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	ret = setsockopt(broadcastsd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));

	bzero(&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = 0x00; //INADDR_ANY = 0
	server.sin_port = htons(BROADCAST_PORT);

	form.sin_family = AF_INET;
	form.sin_addr.s_addr = htonl(INADDR_BROADCAST); //255.255.255.255
	form.sin_port = htons(55550);
	flag = 0;
	do
	{
		ret = bind(broadcastsd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
		if (ret == 0)
			break;
		printf("Broadcast bind Fail...  %d\n", ret);
		sleep(1);
	} while (1);
}
void tcp_create()
{
	int ret, flag;
	struct timeval tv;
	tv.tv_sec = 4;
	tv.tv_usec = 0;

	tcp_sd = socket(AF_INET, SOCK_STREAM, 0);

	do
	{
		tcp_sd = socket(AF_INET, SOCK_STREAM, 0);
		if (tcp_sd > 0)
			break;
		printf("TCP Socket Fail\n");
		sleep(1);
	} while (1);

	printf("test cursor socket tcp_sd = %d ip = %s port = %d \n", tcp_sd, "INADDR_ANY", PORT);
	flag = 1;
	setsockopt(tcp_sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
	setsockopt(tcp_sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	struct sockaddr_in tcp_addr;
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(PORT);
	tcp_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr(ip[1]);

	do
	{
		ret = bind(tcp_sd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr));
		if (ret == 0)
			break;
		printf("TCP bind Fail..%d\n", ret);
		sleep(1);
	} while (1);

	printf("test cursor Bind successfully.\n");

	do
	{
		ret = listen(tcp_sd, 5);
		if (ret >= 0)
			break;
		printf("TCP listen Fail... %d\n", ret);
		sleep(1);
	} while (1);

	printf("Server-Ready for client connection...\n");
}
int receive_data(int sd, char *modelname, char *version, char *devicename)
{
	int ret, calculate_file;
	unsigned char buf[128] = {0};
	unsigned char *rom_buf;
	unsigned int write_length, percentage, tcptotallength;

	PJUVCHDR t6_head;
	PDEVICEINFO t6_info;

	FILE *fp;

	//==============================Receive head===================================
	fp = fopen(FILE_NAME, "w");
	if (fp == NULL)
	{
		printf("File open fail !....\n");
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
	t6_info = (PDEVICEINFO)(buf + 32);
	if (t6_head->XactType == JUVC_TYPE_READY) // 6 ready
	{
		t6_head->XactType == JUVC_TYPE_READY;
		t6_head->Role = JUVC_DEVICE_TYPE_TX;
		t6_head->TotalLength = 96;
		strcpy(t6_info->DeviceName, devicename);
		strcpy(t6_info->ModelName, modelname);
		strcpy(t6_info->Version, version);
		ret = send(sd, buf, t6_head->TotalLength, 0);
		printf("send ret = %d\n", ret);
	}

	bzero(buf, sizeof(buf));
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

		rom_buf = malloc(PACKET_LENGTH_MAX);
		tcptotalget = 0;
		calculate_file = tcptotallength;
		printf("Receive head Error = %d \n", ret);
		printf("Re-receive Broadcast !...........\n");
		while (tcptotallength)
		{
			bzero(rom_buf, PACKET_LENGTH_MAX);
			ret = recv(sd, rom_buf, PACKET_LENGTH_MAX, 0);
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
			t6_head = (PJUVCHDR)buf;
			t6_head->Tag = 1247106627;
			t6_head->XactType = 7; //XactType = 7 (receive status)
			t6_head->HdrSize = 32;
			t6_head->TotalLength = 64;
			memcpy(buf + 32, &percentage, sizeof(percentage));
			ret = send(sd, buf, t6_head->TotalLength, 0);
			printf("send ret = %d\n", ret);
			usleep(10000);
		}
	}
	else
		return -1;
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
int crc_checksum(int sd)
{
	int calculate_CRC, receiveCRC, ret;
	char *file_buffer;
	unsigned char buf[64] = {0};
	PDEVICEINFO t6_info;
	PJUVCHDR t6_head;
	FILE *fp;
	image_header_t head;
	//====================Check CRC and send CRC result==========================
	file_buffer = (char *)malloc(tcptotalget);
	sleep(1);

	fp = fopen(FILE_NAME, "r");
	if (fp == NULL)
	{
		printf("open file error !......\n");
		return -1;
	}
	printf("file_size = %d\n", tcptotalget);
	bzero(file_buffer, tcptotalget);
	fread(file_buffer, 1, tcptotalget, fp);
	t6_info = (PDEVICEINFO)(buf + 32);
	calculate_CRC = crc32(0, (const unsigned char *)file_buffer + sizeof(image_header_t), tcptotalget - sizeof(image_header_t)); //32 file crc
	fclose(fp);

	fp = fopen(FILE_NAME, "r");
	if (fp == NULL)
	{
		printf("open file error !......\n");
		return -1;
	}
	fread(&head, 1, sizeof(image_header_t), fp); //read data CRC
	receiveCRC = ntohl(head.ih_dcrc);
	fclose(fp);

	bzero(buf, sizeof(buf));
	t6_head = (PJUVCHDR)buf;
	t6_head->Tag = 1247106627;
	t6_head->XactType = 4; //XactType = 4 (CRC)
	t6_head->HdrSize = 32;
	t6_head->TotalLength = 64;

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
		ret = send(sd, buf, t6_head->TotalLength, 0);
		printf("send ret = %d\n", ret);
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
		ret = send(sd, buf, sizeof(buf), 0);
		printf("send ret = %d\n", ret);
	}
	sleep(1);
	free(file_buffer);
	return 0;
}
int write_progress(int sd, int kill_rc)
{
	/*============Write progress 、read Program process and send=================*/
	int fpeng;
	unsigned char writebuf[4] = {0};
	unsigned char cmd[512] = {0};
	unsigned char buf[64] = {0};
	PJUVCHDR t6_head;
	FILE *fp_progress;
	int ret = 0;

	printf("File_TotalLength:%d\n", tcptotalget);
	printf("kill_rc = %d\n", kill_rc);
	if (kill_rc != 0)
	{

		snprintf(cmd, sizeof(cmd), "mtd_write -o %d -l %d write %s Kernel &", 0, tcptotalget, FILE_NAME); //FILE_NAME
		//snprintf(cmd, sizeof(cmd), "/home/ryan/samba/updater/mtd_write -o %d -l %d write %s Kernel &", 0, tcptotalget, "/home/ryan/samba/updater/RusbFW");
		system(cmd);
		system("rm /var/mtd_write.log");
		system("touch /var/mtd_write.log");
		usleep(10000);
		progress = 0;
	}

	while (progress < 102)
	{
		fp_progress = fopen("/var/mtd_write.log", "r"); // /var/mtd_write.log  /home/ryan/samba/updater/write_file.txt
		if (fp_progress == NULL)
		{
			printf("File open fail, %d\n", errno);
			return -1;
		}
		fread(writebuf, 1, sizeof(writebuf), fp_progress);
		sscanf(writebuf, "%d", &progress);
		printf("progress : %d\n", progress);
		t6_head = (PJUVCHDR)buf;
		t6_head->Tag = 1247106627;
		t6_head->XactType = JUVC_TYPE_UPDATE_STATUS; //XactType = 3 (write status)
		t6_head->HdrSize = 32;
		t6_head->TotalLength = 64;
		printf("progress : %d   %d\n", progress, __LINE__);
		memcpy(buf + 32, &progress, sizeof(progress));
		ret = send(sd, buf, t6_head->TotalLength, 0);
		printf("ret = %d\n", ret);
		fclose(fp_progress);
		usleep(1000000);
		if (ret == -1)
		{
			return -1;
		}
		if (progress >= 100)
			break;
	}

	return 0;
}
void *thread_fun(void *data)
{ // tcp acceptfd program
	PJUVCHDR t6_head;
	PDEVICEINFO t6_info;
	PTHREADINFO t6_thread;
	//int t6_thread->sd = *(int *)data;
	int ret;
	char buf[128] = {0};
	char led[6] = {0};
	FILE *fp;
	t6_thread = *(PTHREADINFO *)data;
	t6_head = (PJUVCHDR)buf;
	t6_info = (PDEVICEINFO)(buf + 32);
	printf("t6_thread->sd = %d\n", t6_thread->sd);
	printf("t6_thread->flag = %d\n", t6_thread->flag);
	printf("t6_thread->kill_rc = %d\n", t6_thread->kill_rc);
	printf("t6_thread->Version = %s\n", t6_thread->Version);
	printf("t6_thread->ModelName = %s\n", t6_thread->ModelName);
	printf("t6_thread->DeviceName = %s\n", t6_thread->DeviceName);

	if (t6_thread->flag == 0)
	{
		if ((ret = receive_data(t6_thread->sd, t6_thread->ModelName, t6_thread->Version, t6_thread->DeviceName)) == -1)
		{

			t6_head->XactType = 4; //XactType = 4 (CRC status)
			memcpy(buf + 32, "receive_data Error !", 21);
			send(t6_thread->sd, buf, 53, 0);

			fp = fopen("/tmp/led", "r");
			if (fp == NULL)
			{
				printf("open file error !......\n");
				pthread_exit(NULL);
			}
			fread(led, 1, sizeof(led), fp);

			printf("led = %s\n", led);
			if (led != "red")
			{
				system("gpio l 52 0 4000 0 1 4000");
				system("gpio l 14 4000 0 1 0 4000");
				system("echo 'red' > /tmp/led");
			}
			fclose(fp);
			close_socket(t6_thread->sd, tcp_sd, broadcastsd);

			printf("receive_data = %d\n", ret);
			sleep(1);
			pthread_exit(NULL);
		}
		t6_thread->flag++;
	}
	// CRC Checksum
	if (t6_thread->flag == 1)
	{
		if ((ret = crc_checksum(t6_thread->sd)) == -1)
		{
			t6_head->XactType = 4; //XactType = 4 (CRC)
			memcpy(buf + 32, "File Content is Error !", 24);
			send(t6_thread->sd, buf, 56, 0);

			bzero(led, sizeof(led));
			fp = fopen("/tmp/led", "r");
			if (fp == NULL)
			{
				printf("open file error !......\n");
				pthread_exit(NULL);
			}
			fread(led, 1, sizeof(led), fp);

			printf("led = %s\n", led);
			if (led != "red")
			{
				system("gpio l 52 0 4000 0 1 4000");
				system("gpio l 14 4000 0 1 0 4000");
				system("echo 'red' > /tmp/led");
			}
			fclose(fp);
			sleep(1);
			close_socket(t6_thread->sd, tcp_sd, broadcastsd);
			printf("crc_checksum = %d\n", ret);
			pthread_exit(NULL);
		}
		t6_thread->flag++;
	}

	if (t6_thread->flag == 2)
	{
		if ((ret = write_progress(t6_thread->sd, t6_thread->kill_rc)) == -1)
		{
			t6_head->XactType = 4; //XactType = 4 (CRC)
			memcpy(buf + 32, "write_progress Error !", 23);
			send(t6_thread->sd, buf, 55, 0);
			bzero(led, sizeof(led));
			fp = fopen("/tmp/led", "r");
			if (fp == NULL)
			{
				printf("open file error !......\n");
				pthread_exit(NULL);
			}
			fread(led, 1, sizeof(led), fp);

			printf("led = %s\n", led);
			if (led != "red")
			{
				system("gpio l 52 0 4000 0 1 4000");
				system("gpio l 14 4000 0 1 0 4000");
				system("echo 'red' > /tmp/led");
			}
			fclose(fp);
			printf("write_progress = %d\n", ret);
			sleep(1);
			pthread_exit(NULL);
		}
	}
	close_socket(t6_thread->sd, tcp_sd, broadcastsd);
	printf(KGRN "Update Complete!");
	printf("\n" RESET);
	system("gpio l 14 0 4000 0 1 3000");
	system("gpio l 52 4000 0 1 0 4000");

	//system("killall -SIGTTIN nvram_daemon");
	//system("reboot");
	pthread_detach(pthread_self());
	pthread_exit(NULL);
}
void *thread_write(void *data)
{ // tcp acceptfd program
	PJUVCHDR t6_head;
	PDEVICEINFO t6_info;
	PTHREADINFO t6_thread;
	//int t6_thread->sd = *(int *)data;
	int ret;
	char buf[128] = {0};
	char led[6] = {0};
	FILE *fp;
	t6_thread = *(PTHREADINFO *)data;
	t6_head = (PJUVCHDR)buf;
	t6_info = (PDEVICEINFO)(buf + 32);
	printf("t6_thread->sd = %d\n", t6_thread->sd);
	printf("t6_thread->flag = %d\n", t6_thread->flag);
	printf("t6_thread->kill_rc = %d\n", t6_thread->kill_rc);
	printf("t6_thread->Version = %s\n", t6_thread->Version);
	printf("t6_thread->ModelName = %s\n", t6_thread->ModelName);
	printf("t6_thread->DeviceName = %s\n", t6_thread->DeviceName);

	if ((ret = write_progress(t6_thread->sd, t6_thread->kill_rc)) == -1)
	{
		t6_head->XactType = 4; //XactType = 4 (CRC)
		memcpy(buf + 32, "write_progress Error !", 23);
		send(t6_thread->sd, buf, 55, 0);
		bzero(led, sizeof(led));
		fp = fopen("/tmp/led", "r");
		if (fp == NULL)
		{
			printf("open file error !......\n");
			pthread_exit(NULL);
		}
		fread(led, 1, sizeof(led), fp);

		printf("led = %s\n", led);
		if (led != "red")
		{
			system("gpio l 52 0 4000 0 1 4000");
			system("gpio l 14 4000 0 1 0 4000");
			system("echo 'red' > /tmp/led");
		}
		fclose(fp);
		printf("write_progress = %d\n", ret);
		sleep(1);
		pthread_exit(NULL);
	}

	close_socket(t6_thread->sd, tcp_sd, broadcastsd);
	printf(KGRN "Update Complete!");
	printf("\n" RESET);
	system("gpio l 14 0 4000 0 1 3000");
	system("gpio l 52 4000 0 1 0 4000");

	//system("killall -SIGTTIN nvram_daemon");
	//system("reboot");
	pthread_detach(pthread_self());
	pthread_exit(NULL);
}
int main(int argc, char *avg[])
{
	int recvbytes, sendBytes, fromlen, ret, acceptfd, kill_rc;
	unsigned int sin_size, write_length, maxfd;
	unsigned char buf[128] = {0};
	char thread_data[64] = {0};
	char macaddr[7] = {0};
	char led[7] = {0};
	char *modelname, *version, *devicename;
	unsigned char writebuf[4] = {0};
	pthread_t thread_1, thread_2;
	/*TEST
	modelname = malloc(strlen("IPW611"));
	version = malloc(strlen("1.0.0.210428"));
	devicename = malloc(strlen("DEVICENAME"));
	strcpy(modelname, "IPW611");
	strcpy(version, "1.0.0.210428");
	strcpy(devicename, "DEVICE_NAME");
	memcpy(macaddr, get_macaddr("eth0"), 6); //rai0
	*/
	PJUVCHDR t6_head;
	PDEVICEINFO t6_info;
	PTHREADINFO t6_thread;
	struct timeval tv;
	struct sockaddr_in their_addr;
	FILE *fp, *fp_progress;

	broadcast_create();
	tcp_create();

	ret = 0;
	acceptfd = 0;
	printf("Waiting Receive BroadCast !.......  ret = %d\n", ret);
	fd_set read_sd;
	FD_ZERO(&read_sd);
	signal(SIGPIPE, SIG_IGN);

	kill_rc = 1;

	memcpy(macaddr, get_macaddr("rai0"), 6);

	bzero(thread_data, sizeof(thread_data));
	fp = popen("nvram_get ModelName", "r");
	fgets(thread_data, sizeof(thread_data), fp);
	modelname = malloc(strlen(thread_data));
	strncpy(modelname, thread_data, strlen(thread_data));
	pclose(fp);

	fp = popen("nvram_get Version", "r");
	fgets(thread_data, sizeof(thread_data), fp);
	version = malloc(strlen(thread_data));
	strncpy(version, thread_data, strlen(thread_data));
	pclose(fp);

	fp = popen("nvram_get HostName", "r");
	fgets(thread_data, sizeof(thread_data), fp);
	devicename = malloc(strlen(thread_data));
	strncpy(devicename, thread_data, strlen(thread_data));
	pclose(fp);

	while (1)
	{
		FD_SET(tcp_sd, &read_sd);
		FD_SET(broadcastsd, &read_sd);
		tv.tv_sec = 7;
		tv.tv_usec = 0;
		maxfd = (tcp_sd > broadcastsd) ? (tcp_sd + 1) : (broadcastsd + 1); //which max+1
		ret = select(maxfd + 1, &read_sd, 0, 0, &tv);

		if (ret == 0) //timeout
		{
			printf("Waiting Receive BroadCast !.......  ret = %d\n", ret);
		}
		else if (ret == -1)
		{ //fail
			printf("Select Error Re-receive Broadcast !...... ret = %d\n", ret);
			close_socket();
			broadcast_create();
			if (acceptfd > 0)
			{
				kill_rc = pthread_kill(thread_1, 0);
				printf("kill_rc = %d\n", kill_rc);
			}
			/*if (kill_rc != 0) // = 0 alive
			{
				tcp_create();
			}*/
			tcp_create();
			FD_ZERO(&read_sd);
			kill_rc = 1;
			ret = 0;
		}
		// add tcp connect
		else if (FD_ISSET(tcp_sd, &read_sd))
		{
			system("killall apclient_chkconn.sh 1>/dev/null 2>/dev/null");
			sin_size = sizeof(struct sockaddr_in);
			acceptfd = accept(tcp_sd, (struct sockaddr *)&their_addr, &sin_size);
			if (acceptfd < 0)
			{
				printf("Server-accept() Error Re-receive Broadcast !......\n");
				//close_socket(0, tcp_sd, 0);
				//acceptfd = 0;
				continue;
			}
			printf("Server-accept() is OK\n");
			printf("Server-new socket, acceptfd is OK...\n");
			printf("Got connection from the client: %s\n", inet_ntoa(their_addr.sin_addr)); // *client IP

			FD_SET(acceptfd, &read_sd); // add master set
			if (acceptfd >= maxfd)
			{ // max fd
				maxfd = acceptfd;
			}
			fp = fopen("/tmp/led", "r");
			if (fp == NULL)
			{
				printf("open file error !......\n");
				return -1;
			}
			fread(led, 1, sizeof(led), fp);

			printf("led = %s\n", led);
			if (strcmp(led, "flash") != 0)
			{
				system("gpio l 52 0 4000 0 1 4000");
				system("gpio l 14 1 1 1 1 4000");
				system("echo 'flash' > /tmp/led");
			}
			fclose(fp);

			bzero(thread_data, sizeof(thread_data));
			t6_thread = (PTHREADINFO)thread_data;
			t6_thread->sd = acceptfd;
			t6_thread->flag = 2;
			strncpy(t6_thread->DeviceName, devicename, strlen(devicename) - 1);
			strncpy(t6_thread->ModelName, modelname, strlen(modelname) - 1);
			strncpy(t6_thread->Version, version, strlen(version) - 1);
			fp = fopen("/var/mtd_write.log", "r"); // /var/mtd_write.log  /home/ryan/samba/updater/write_file.txt
			if (fp == NULL)
			{
				if ((ret = receive_data(t6_thread->sd, t6_thread->ModelName, t6_thread->Version, t6_thread->DeviceName)) == -1)
				{

					t6_head->XactType = 4; //XactType = 4 (CRC status)
					memcpy(buf + 32, "receive_data Error !", 21);
					send(t6_thread->sd, buf, 53, 0);

					fp = fopen("/tmp/led", "r");
					if (fp == NULL)
					{
						printf("open file error !......\n");
						pthread_exit(NULL);
					}
					fread(led, 1, sizeof(led), fp);

					printf("led = %s\n", led);
					if (led != "red")
					{
						system("gpio l 52 0 4000 0 1 4000");
						system("gpio l 14 4000 0 1 0 4000");
						system("echo 'red' > /tmp/led");
					}
					fclose(fp);
					close_socket(t6_thread->sd, tcp_sd, broadcastsd);

					printf("receive_data = %d\n", ret);
					continue;
				}
				if ((ret = crc_checksum(t6_thread->sd)) == -1)
				{
					t6_head->XactType = 4; //XactType = 4 (CRC)
					memcpy(buf + 32, "File Content is Error !", 24);
					send(t6_thread->sd, buf, 56, 0);

					bzero(led, sizeof(led));
					fp = fopen("/tmp/led", "r");
					if (fp == NULL)
					{
						printf("open file error !......\n");
						pthread_exit(NULL);
					}
					fread(led, 1, sizeof(led), fp);

					printf("led = %s\n", led);
					if (led != "red")
					{
						system("gpio l 52 0 4000 0 1 4000");
						system("gpio l 14 4000 0 1 0 4000");
						system("echo 'red' > /tmp/led");
					}
					fclose(fp);
					sleep(1);
					close_socket(t6_thread->sd, tcp_sd, broadcastsd);
					printf("crc_checksum = %d\n", ret);
					continue;
				}
				pthread_create(&thread_2, NULL, thread_write, &t6_thread);
				while (100 || socket write failed)
				{
					get progress from file and send back to AP
				}
			}
			/*
			kill_rc = pthread_kill(thread_1, 0);
			printf("kill_rc = %d\n", kill_rc);
			if (kill_rc == 0)
			{
				t6_thread->kill_rc = kill_rc;
				pthread_create(&thread_1, NULL, thread_fun, &t6_thread);
			}
			else
			{
				t6_thread->flag = 0;
				t6_thread->kill_rc = 1;
				pthread_create(&thread_1, NULL, thread_fun, &t6_thread);
			}
			kill_rc = pthread_kill(thread_1, 0);
			printf("kill_rc = %d\n", kill_rc);
			*/
			else
			{
				//pthread_create(&thread_2, NULL, thread_write, &t6_thread);
				while (100 || socket write failed)
				{
					get progress from file and send back to AP
				}
			}
			fclose(fp);
		}

		// *Broadcast
		else if (FD_ISSET(broadcastsd, &read_sd))
		{
			fromlen = sizeof(struct sockaddr_in);
			bzero(buf, sizeof(buf));
			recvfrom(broadcastsd, buf, 32, 0, (struct sockaddr *)&server, &fromlen);
			t6_head = (PJUVCHDR)buf;

			if (t6_head->XactType == JUVC_TYPE_DISCOVER) //JUVC_TYPE_DISCOVER =5
			{
				t6_head = (PJUVCHDR)buf;
				t6_info = (PDEVICEINFO)(buf + 32);
				t6_head->Tag = 1247106627;
				t6_head->XactType = JUVC_TYPE_INFO;
				t6_head->Role = JUVC_DEVICE_TYPE_TX; //TX or RX select
				t6_head->XactId = 204;
				t6_head->HdrSize = 32;
				t6_head->TotalLength = 96;
				t6_info->Port = 55551;
				t6_info->Manufacture = 1; //MCT
				memcpy(t6_info->MacAddress, macaddr, 6);
				strncpy(t6_info->ModelName, modelname, strlen(modelname) - 1);
				strncpy(t6_info->Version, version, strlen(version) - 1);
				strncpy(t6_info->DeviceName, devicename, strlen(devicename) - 1);
				sendBytes = sendto(broadcastsd, buf, t6_head->TotalLength, 0, (struct sockaddr *)&form, fromlen);
				printmessage(t6_head, t6_info);
			}
		}
	}
}
