

#ifndef _jwr2100_tcp_cmd_def_h_
#define _jwr2100_tcp_cmd_def_h_

#define JWR2100_TAG                      0x504A5752 //'PJWR'
#define JWR_MAX_PACKET_SIZE              1024


#define JWR_BROADCAST_PORT  22667
#define JWR_TCP_PORT        22668 
#define JWR_UDP_PORT_CURSOR 22669
#define JWR_UDP_PORT_AUDIO  22670
#define JWR_ROM_TCP_PORT    22800 
#define MAC_TCP_VIDEO_PORT  70000

#define SW_DISP             4 
#define SW_DISP_6           6 
#define JWR_CMD_RES_LIST             0x00  //server => client
#define JWR_CMD_UDP_PORT             0x01  //server => client  


#define JWR_CMD_RES_MAX              0x02  //client => server 
#define JWR_CMD_EVENT_INACTIVE       0x03  //client => server  
#define JWR_CMD_EVENT_ACTIVE         0x04  //client => server 
#define JWR_CMD_EVENT_DISCONNECT     0x05  //client => server  
#define JWR_CMD_EVENT_CONNECT        0x09  //client => server 
#define JWR_CMD_EVENT_SHIFT_LEFT        0x0a  //client => server 
#define JWR_CMD_EVENT_SHIFT_RIGHT      0x0b  //client => server 

#define JWR_CMD_ACTION_PREVIEW       0x06  //server => client  data => struct tCAPTURE_PREVIEW
#define JWR_CMD_ACTION_INACTIVE      0x07  //server => client
#define JWR_CMD_ACTION_ACTIVE        0x08  //server => client  data => struct tCAPTURE_ACTIVE




#define JWR_CMD_SET_ROM_WRITE            0x20
#define JWR_CMD_GET_ROM_PROCESS          0x21
#define JWR_CMD_RESET                    0x22 

#define RAM_ADDR_GAP       (1024*512)
#define MAX_FBRAM_ADDR     (1024*1024*50)
#define MAX_AFB1RAM_ADDR   (1024*1024*38)
#define MAX_AFB2RAM_ADDR   (1024*1024*48)

#pragma pack(push, 1)


typedef struct tCAPTURE_ACTIVE{
    unsigned short ActiveWidth;
	unsigned short ActivewHeight;
}CAPTURE_ACTIVE,*PCAPTURE_ACTIVE;

typedef struct tCAPTURE_PREVIEW{
	unsigned short PreviewWidth;
	unsigned short PreviewHeight;
	int x;
	int y;
}CAPTURE_PREVIEW,*PCAPTURE_PREVIEW;

typedef struct tCAPTURE_EVENT{
	unsigned char  event;
}CAPTURE_EVENT,*PCAPTURE_EVENT;

typedef struct tJWR_UDP_PORT{
	unsigned int  video_port;
	unsigned int  audio_port;
	unsigned int  cursor_port;
}JWR_UDP_PORT,*PJWR_UDP_PORT;

/*****************************************************************************
 * Jwr2100 Transaction Header
 *****************************************************************************
 * 
 */
typedef struct tJwrCmdHdr
{
    unsigned int        Tag;                   
    unsigned char       cmd;               
    unsigned short      DataLength;             
    unsigned char       Rsvd[17];            
} JWRCMDHDR, *PJWRCMDHDR;

typedef struct tJwrCmdRomHdr
{
    unsigned int        Tag;                   
    unsigned char       cmd;               
    unsigned int        DataLength;             
    unsigned char       Rsvd[15];            
} JWRCMDROMHDR, *PJWRCMDROMHDR;

#pragma pack(pop)
#endif