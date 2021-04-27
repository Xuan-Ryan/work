#ifndef display_server_h
#define display_server_h
#include "queue.h"

#define DEBUG

#define BROADCAST_PORT  22667
#define TCP_PORT        22668 
#define UDP_PORT_AUDIO  22670
#define UDP_PORT_HID    22671
#define JWR_TCP_PORT_CURSOR    22672
#define USB_TCP_PORT    22690

#define UDP_MAX_BUFFER  8388608 
#define TCP_MAX_BUFFER  8388608 
#define QUEUE_MAX       1477
#define MAX_DISP_USE  9
#define TCP_MAX_PACKET  1024
#define PREVIEM_TMP_SIZE 524288
typedef enum
{
	UDP_PORT_VIDEO_P1  =      22681, 
	UDP_PORT_VIDEO_P2  , 
	UDP_PORT_VIDEO_P3  , 
	UDP_PORT_VIDEO_P4  , 
	UDP_PORT_VIDEO_P5  , 
	UDP_PORT_VIDEO_P6  , 
	UDP_PORT_VIDEO_P7  , 
	UDP_PORT_VIDEO_P8  , 
	UDP_PORT_VIDEO_P9  , 

}UDP_PORT_VIDEO_BASE;


#define SERVER_IP_NAME  "br0"
//#define SERVER_IP_NAME  "eth0"


#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


#pragma pack(push, 1)



struct display_ip
{
    unsigned int ipaddr;
	int tcp_socket_fd ;
	int udp_socket_fd ;
	int udp_port_num  ;
	int tcp_connection  ;
	int upd_connection_r ;
	int upd_connection_p;
	int x;
	int y;
	int preview_w ;
	int preview_h ;
	int active_w ;
	int active_h ;
	int client_res_w ;
	int client_res_h ;
	int action ;
	int restart ;
	int position;
	//int preveiw_length;
	//char preveiw_temp[PREVIEM_TMP_SIZE]; 
	queue_t* Queue_image_requests;
    struct display_ip *next;
};

struct hidpendev
{
	int fd;
    int detect;
	int thread_run;
	int *socket;
};

#pragma pack(pop)

int g_usb_fd = -1 ;
#endif
