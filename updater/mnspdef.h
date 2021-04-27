/*****************************************************************************
 * mnspdef.h
 *****************************************************************************
 * Copyright (c) MCT Corporation.  All rights reserved.
 * 
 * Module Name:
 *      mnspdef.h
 *
 * Abstract:
 *      Defines the MCT Network Streaming Protocol.
 * 
 * Environment:
 *      User mode
 *
 * Revision History:
 *      2016/11/08 Feng, Chun-Chiang (ccfeng@mct.com.tw)
 *          1) First Release.
 *
 */

#ifndef _mnsp_def_h_
#define _mnsp_def_h_

//#ifndef INT8
//typedef signed char                             INT8, *PINT8;
//typedef signed short                            INT16, *PINT16;
//typedef signed int                              INT32, *PINT32;
//typedef int64_t                                 INT64, *PINT64;
//typedef unsigned char                           UINT8, *PUINT8;
//typedef unsigned short                          UINT16, *PUINT16;
//typedef unsigned int                            UINT32, *PUINT32;
//typedef uint64_t                                UINT64, *PUINT64;
//#endif

#define MNSP_TAG 1347636813 //'PSNM'

// MNSP_XACTTYPE_XXX
#define MNSP_XACTTYPE_CMD 0x00     // Command transaction.
#define MNSP_XACTTYPE_VIDEO 0x01   // Video transaction.
#define MNSP_XACTTYPE_CURSOR 0x02  // Cursor transaction.
#define MNSP_XACTTYPE_SPEAKER 0x03 // Speaker transaction.
#define MNSP_XACTTYPE_ALIVE 0x04   // Alive transaction.

#define MNSP_MULTICAST_BASE_IP ((226 << 24) | (1 << 16) | (1 << 8) | 0)
#define MNSP_MULTICAST_STA_IP(_idx_) (MNSP_MULTICAST_BASE_IP | (_idx_)) // must be one-based.

#define MNSP_BASE_PORT 22660
#define MNSP_CMD_PORT (MNSP_BASE_PORT + MNSP_XACTTYPE_CMD)         // TCP
#define MNSP_VIDEO_PORT (MNSP_BASE_PORT + MNSP_XACTTYPE_VIDEO)     // MCast
#define MNSP_CURSOR_PORT (MNSP_BASE_PORT + MNSP_XACTTYPE_CURSOR)   // MCast
#define MNSP_SPEAKER_PORT (MNSP_BASE_PORT + MNSP_XACTTYPE_SPEAKER) // MCast

#define MNSP_MAX_MCAST_PACKET_SIZE 60 * 1024 //1420

#pragma pack(push, 1)

/*****************************************************************************
 * MNSP Transaction Header
 *****************************************************************************
 * 
 */
typedef struct tMnspXactHdr
{
    unsigned int Tag;           // 0000h Protocol tag. Must be MNSP_TAG.
    unsigned char XactType;     // 0004h Transaction type. See MNSP_XACTTYPE_XXX.
    unsigned char HdrSize;      // 0005h Header size, in byte. It means the payload start.
    unsigned char XactId;       // 0006h Transaction Id. Range: 0 ~ 255.
    unsigned char Flags;        // 0007h Flags.
    unsigned int PayloadLength; // 0008h Payload length, in byte.
    //unsigned char               Rsvd_0Ah[2];            // 000Ah Reserved. maybe payload length is not enough for command (tcp)
    unsigned int TotalLength; // 000Ch Total data length of the transaction
    unsigned int XactOffset;  // 0010h Data offset at this transaction
    unsigned int CurrentnetBps;
    unsigned char Rsvd_14h[8]; // 0014h-1Fh Reserved.
} MNSPXACTHDR, *PMNSPXACTHDR;

typedef struct tWifiDongleRomHdr
{
    unsigned char FW_for_dest; //FW for destination.
    unsigned char HdrSize;     //Header size.
    unsigned char Flag1;
    unsigned char Flag2;
    unsigned int PayloadLength; //FW size.
    unsigned char Rsvd_8h[24];
} WIFIDONGLEROMHDR, *PWIFIDONGLEROMHDR;

enum wifi_dongle_dest
{
    FW_DEST_TX = 0,
    FW_DEST_RX,
    FW_DEST_T6
};

#pragma pack(pop)

#endif
