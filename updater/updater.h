/*****************************************************************************
 * updater.h
 *****************************************************************************
 * Copyright (c) MCT Corporation.  All rights reserved.
 * 
 * Module Name:
 *      updater.h
 *
 * Abstract:
 *      Defines the T6
 * 
 * Environment:
 *      User / Kernel / Firmware mode
 *
 * Revision History:
 *      2021/04/20 ryan(ryan0921@mct.com.tw)
 *          1) First Release.
 *
 */

// ============================================================================

/*  ----------------  protocol  ----------------  */

#define JUVC_TAG                            1247106627 /* JUVC  */
/*  JUVC_TYPE_XXX  */
#define JUVC_TYPE_NONE                      0
#define JUVC_TYPE_CONTROL                   1
#define JUVC_TYPE_CAMERA                    2
#define JUVC_TYPE_MIC                       3
#define JUVC_TYPE_SPK                       4
#define JUVC_TYPE_FIRMWARE                  5
#define JUVC_TYPE_BROADCAST                 254
#define JUVC_TYPE_ALIVE                     255
#define JUVC_TYPR_PORT                      55551

/*  JUVC_FLAGS_XXX  */
#define JUVC_FLAG_NONE                      0
#define JUVC_FW_INFO                        1
#define JUVC_FW_ACK                         2
#define JUVC_DEVICE_TYPE                    3


#define JUVC_DEVICE_TYPE_RX                 0           /* Header->XactType */
#define JUVC_DEVICE_TYPE_TX                 1           /* Header->XactType */


/* john JUVC_TYPE_XXX  */
#define JUVC_TYPE_INFO                      1
#define JUVC_TYPE_UPDATE_DATA               2           /* Header->XactType */
#define JUVC_TYPE_UPDATE_STATUS             3
#define JUVC_TYPE_CRC_CHECK                 4
#define JUVC_TYPE_DISCOVER                  5           /* Header->XactType */
#define JUVC_TYPE_READY                     6           /* Header->XactType */
#define JUVC_TYPE_RECEIVE_STATUS            7
#define JUVC_TYPE_BUSY_STATUS               8


typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef uint64_t UINT64, *PUINT64;

#define IH_MAGIC                            0x27051956
#define IH_NMLEN                            32 - 4

/*  not ready now */
//typedef struct juvc_client_info
//{
//    unsigned char mac_addr[18]; /*  mac address 00:05:1B:A0:00:00  */
//} JUVCCLNTINFO, *PJUVCCLNTINFO;
#pragma pack(0)
typedef struct image_header
{
    uint32_t ih_magic;              /* Image Header Magic Number    */
    uint32_t ih_hcrc;               /* Image Header CRC Checksum    */
    uint32_t ih_time;               /* Image Creation Timestamp */
    uint32_t ih_size;               /* Image Data Size      */
    uint32_t ih_load;               /* Data  Load  Address      */
    uint32_t ih_ep;                 /* Entry Point Address      */
    uint32_t ih_dcrc;               /* Image Data CRC Checksum  */
    uint8_t ih_os;                  /* Operating System     */
    uint8_t ih_arch;                /* CPU architecture     */
    uint8_t ih_type;                /* Image Type           */
    uint8_t ih_comp;                /* Compression Type     */
    uint8_t ih_name[IH_NMLEN];      /* Image Name       */
    uint32_t ih_ksz;                /* Kernel Part Size              */
} image_header_t;

/*  header size is 32 bytes  */
typedef struct juvc_hdr_packet
{
    unsigned int Tag;                   /*  JUVC  */
    unsigned int XactType       : 4;    /*  JUVC_TYPE_XXX  */
    unsigned int Role           : 4;    /*TX or RX   Flags  */
    unsigned int XactId         : 8;
    unsigned int Rsv            : 16;
    unsigned int HdrSize;           /*  should be 32 always  */
    //unsigned char Flags;          /*  JUVC_FLAGS_XXX or JUVC_CONTROL_XXX  */
    //unsigned int PayloadLength;
    unsigned int TotalLength;
    unsigned char Rsvd[16];
} JUVCHDR, *PJUVCHDR;

typedef struct deviceinfo
{   
    UINT32          IPaddress;
    UINT8           MacAddress[8];             /* mac address 00:05:1B:A0:00:00  */
    UINT32          Port;                      /* TCP connceting port */
    UINT32          Manufacture;               /* MCT = 1 */
    UINT8           ModelName[12];             /*modelname = IPW611T/R */
    UINT8           Version[16];               /*IPW611T/R-  0.0.0.210427.bin*/
    UINT8           DeviceName[16];            /*devicename =TEST_NAME */  
} DEVICEINFO, *PDEVICEINFO;

typedef struct threadinfo
{
    UINT32          sd;
    UINT8           ModelName[12];             /*modelname = IPW611T/R */
    UINT8           Version[16];               /*IPW611T/R-  0.0.0.210427.bin*/
    UINT8           DeviceName[16];            /*devicename =TEST_NAME */       
    UINT8           Rsvd[22];
} THREADINFO, *PTHREADINFO;
