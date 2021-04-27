/*****************************************************************************
 * t6bulkdef.h
 *****************************************************************************
 * Copyright (c) MCT Corporation.  All rights reserved.
 * 
 * Module Name:
 *      t6bulkdef.h
 *
 * Abstract:
 *      Defines the T6 bulk & interrupt transfer.
 * 
 * Environment:
 *      User / Kernel / Firmware mode
 *
 * Revision History:
 *      2016/04/11 Feng, Chun-Chiang (ccfeng@mct.com.tw)
 *          1) First Release.
 *      2016/04/20 Feng, Chun-Chiang (ccfeng@mct.com.tw)
 *          1) Added T6INT_ROM_EVENT_XXX for status monitoring.
 *      2016/06/15 Feng, Chun-Chiang (ccfeng@mct.com.tw)
 *          1) Added AudioRenderJack0 for audio render jack detection.
 *      2016/07/27 Feng, Chun-Chiang (ccfeng@mct.com.tw)
 *          1) Added SIGNATURE_IN, SIGNATURE_AUDIO_IN, SIGNATURE_ROM_IN.
 *      2016/09/13 CC & Ivan
 *          1) Added AudioRenderDVOH, AudioRenderLVDSH, AudioCaptureJackMic, AudioCaptureDataIn.
 *		2017/04/27	Yulw
 *			1) Add T6_DISPLAY_EXT_FLIP_XXX and T6_DISPLAY_EXT_CLIP_XXX extension IDs
 *			2) Modify structure tT6BulkDmaHdr.FuncSpecific.Display for new Extention IDs.
 *
 */

#ifndef _T6_BULK_DEF_H_
#define _T6_BULK_DEF_H_

#define SIGNATURE_DISPLAY 0x00
#define SIGNATURE_USB_HOST 0x01
#define SIGNATURE_NETWORK 0x02
#define SIGNATURE_AUDIO 0x03
#define SIGNATURE_CAMERA 0x04
#define SIGNATURE_ROM 0x05
#define SIGNATURE_STORAGE 0x06
#define SIGNATURE_NV12DMA 0x07

#define SIGNATURE_IN 0x80
#define SIGNATURE_AUDIO_IN (SIGNATURE_IN | SIGNATURE_AUDIO)
#define SIGNATURE_ROM_IN (SIGNATURE_IN | SIGNATURE_ROM)

// ============================================================================
// Bulk-Out Transfer
//

// T6_PACKET_FLAG_XXX
#define T6_PACKET_FLAG_NONE 0x00
#define T6_PACKET_FLAG_CONTINUETOSEND 0x01

// T6_ROM_FLAGS_XXX
#define T6_ROM_FLAGS_NONE 0x00
#define T6_ROM_FLAGS_DONE_THEN_RESET 0x01 // after burned, reset hw
#define T6_ROM_FLAGS_INTERRUPT 0x02       // interrupt with fence id.

// T6_ROM_VERB_XXX
#define T6_ROM_VERB_BURN_IMAGE2 0x00 // 0x00: burn to image-2 region;
#define T6_ROM_VERB_BURN_IMAGE1 0x01 // 0x01: burn to image-1 region;
#define T6_ROM_VERB_BURN_BOOT 0x02   // 0x02: burn to boot region.

//T6_DISPLAY_EXT_XXX
#define T6_DISPLAY_EXT_NONE 0x00
#define T6_DISPLAY_EXT_SET_CURSOR1_SHPAE 0x01 // parameter1: cursor index
#define T6_DISPLAY_EXT_SET_CURSOR2_SHPAE 0x02 // parameter1: cursor index
#define T6_DISPLAY_EXT_FLIP_PRIMARY 0x03      //
#define T6_DISPLAY_EXT_FLIP_SECONDARY 0x04    //
#define T6_DISPLAY_EXT_CLIP_PRIMARY 0x05      //
#define T6_DISPLAY_EXT_CLIP_SECONDARY 0x06    //

#pragma pack(push, 1)

/*  ----------------  protocol  ----------------  */

#define JUVC_TAG 1247106627 /* JUVC  */
/*  JUVC_TYPE_XXX  */
#define JUVC_TYPE_NONE 0
#define JUVC_TYPE_CONTROL 1
#define JUVC_TYPE_CAMERA 2
#define JUVC_TYPE_MIC 3
#define JUVC_TYPE_SPK 4
#define JUVC_TYPE_FIRMWARE 5
#define JUVC_TYPE_BROADCAST 254
#define JUVC_TYPE_ALIVE 255
#define JUVC_TYPR_PORT 55551

/*  JUVC_FLAGS_XXX  */
#define JUVC_FLAG_NONE 0
#define JUVC_FW_INFO 1
#define JUVC_FW_ACK 2
#define JUVC_DEVICE_TYPE 3

#define JUVC_DEVICE_TYPE_RX 0
#define JUVC_DEVICE_TYPE_TX 1

/*  JUVC_CONTROL_XXX  */
#define JUVC_CONTROL_NONE 0
#define JUVC_CONTROL_LOOKUP 1
#define JUVC_CONTROL_CLNTINFO 2
#define JUVC_CONTROL_CAMERAINFO 3
#define JUVC_CONTROL_CAMERACTRL 4
#define JUVC_CONTROL_MANUFACTURER 5
#define JUVC_CONTROL_PRODUCT 6
#define JUVC_CONTROL_DESCRIPTOR 7
#define JUVC_CONTROL_SET 8
#define JUVC_CONTROL_GET 9
#define JUVC_CONTROL_GETVC 10
#define JUVC_CONTROL_STOP_START 11

/* john JUVC_TYPE_XXX  */
#define JUVC_TYPE_INFO              1
#define JUVC_TYPE_UPDATE_DATA       2
#define JUVC_TYPE_UPDATE_STATUS     3
#define JUVC_TYPE_CRC_CHECK         4
#define JUVC_TYPE_DISCOVER          5


/*  JUVC_RES_XXX  */

#define JUVC_RESPONSE_ACK 1
/*  JUVC_CONTROL_CLNTINFO  */

/*  not ready now  */
typedef struct juvc_client_info
{
    unsigned char mac_addr[18]; /*  mac address 00:05:1B:A0:00:00  */
} JUVCCLNTINFO, *PJUVCCLNTINFO;

typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint64_t;
typedef uint64_t UINT64, *PUINT64;

#define IH_MAGIC 0x27051956
#define IH_NMLEN 32 - 4

typedef struct image_header
{
    uint32_t ih_magic;         /* Image Header Magic Number    */
    uint32_t ih_hcrc;          /* Image Header CRC Checksum    */
    uint32_t ih_time;          /* Image Creation Timestamp */
    uint32_t ih_size;          /* Image Data Size      */
    uint32_t ih_load;          /* Data  Load  Address      */
    uint32_t ih_ep;            /* Entry Point Address      */
    uint32_t ih_dcrc;          /* Image Data CRC Checksum  */
    uint8_t ih_os;             /* Operating System     */
    uint8_t ih_arch;           /* CPU architecture     */
    uint8_t ih_type;           /* Image Type           */
    uint8_t ih_comp;           /* Compression Type     */
    uint8_t ih_name[IH_NMLEN]; /* Image Name       */
    uint32_t ih_ksz;           /* Kernel Part Size              */
} image_header_t;

/*  header size is 32 bytes  */
typedef struct juvc_hdr_packet
{
    unsigned int Tag;          /*  JUVC  */
    unsigned int XactType : 4; /*  JUVC_TYPE_XXX  */
    unsigned int Role : 4;     //TX or RX   Flags
    unsigned int XactId : 8;
    unsigned int Rsv : 16;
    unsigned int HdrSize; /*  should be 32 always  */
    //unsigned char Flags; /*  JUVC_FLAGS_XXX or JUVC_CONTROL_XXX  */
    //unsigned int PayloadLength;
    unsigned int TotalLength;
    unsigned char Rsvd[16];
} /* __attribute__((packed))*/ JUVCHDR, *PJUVCHDR;

typedef struct cameractrl
{ /*  for JUVC_CONTROL_CAMERACTRL  */
    unsigned char formatidx;
    unsigned char frameidx;
    unsigned char Rsvd_[2];
    unsigned int camera;
    unsigned int process;
    unsigned int Resv[5];
} /*__attribute__((packed))*/ CAMERACTRL, *PCAMERACTRL;

typedef struct camerainfo
{ /*  for JUVC_CONTROL_CAMERAINFO  */
    unsigned char mjpeg_res_bitfield;
    unsigned char h264_res_bitfield;
    unsigned char yuv_res_bitfield;
    unsigned char nv12_res_bitfield;
    unsigned char i420_res_bitfield;
    unsigned char m420_res_bitfield;
    unsigned char camera[3];
    unsigned char process[3];
    unsigned char Rsvd[20];
} /*__attribute__((packed))*/ CMAERAINFO, *PCAMERAINFO;

typedef struct deviceinfo
{
    unsigned int crc32;
    unsigned int device_type;
    unsigned int port;
    unsigned short idVendor;
    unsigned short idProduct;
    unsigned char name[16];
    //unsigned char MAC_addr[6]
} /*__attribute__((packed))*/ DEVICEINFO, *PDEVICEINFO;

#pragma pack(0)
//===================================================================================================
typedef struct tT6BulkDmaHdr
{
    UINT32 Signature;
    UINT32 PayloadLength;
    UINT32 PayloadAddress;
    UINT32 PacketSize;
    UINT32 PacketOffset;
    UINT8 PacketFlags; // see T6_PACKET_FLAG_XXX.
    UINT8 PacketRsvd[3];
    union
    { // function specific (8 bytes)
        UINT64 U64;
        UINT8 U8[8];

        struct
        {
            UINT8 DisplayExtID; // see T6_DISPLAY_EXT_XXXX
            union
            {
                struct
                {
                    UINT8 Parameter1; // depends on DisplayExtID
                    UINT8 Rsvd[6];    // reserved.
                } ExtCursor;
                struct
                {
                    UINT32 Y_StartAddresss;
                    UINT8 Rsvd[3];
                } ExtFlipClip;
                UINT8 Rsvd[7];
            } ExtParams;
        } Display;

        struct
        {
            UINT8 Flags;       // 0x00: 0x01:restart engine.
            UINT8 EngineIndex; // index of engine.
            UINT8 Rsvd[6];     // reserved.
        } Audio;

        struct
        {
            UINT8 Flags;        // see T6_ROM_FLAGS_XXX
            UINT8 Verb;         // see T6_ROM_VERB_XXX
            UINT16 FenceId;     // fence id. D0-D7:seq_no D8-D15:signature
            UINT32 StartOffset; // start offset from region
        } Rom;
    } FuncSpecific;
} T6BULKDMAHDR;

#pragma pack(pop)

// ============================================================================
// Interrupt-In Transfer
//

// T6INT_FUNC_MASK_XXX
#define T6INT_FUNC_MASK_CPU 0x02
#define T6INT_FUNC_MASK_DISPLAY 0x04
#define T6INT_FUNC_MASK_USB_HOST 0x08
#define T6INT_FUNC_MASK_NETWORK 0x10
#define T6INT_FUNC_MASK_AUDIO 0x20
#define T6INT_FUNC_MASK_SERIAL 0x40
#define T6INT_FUNC_MASK_ROM 0x80

// T6INT_DISP_EVENT_XXX
#define T6INT_DISP_EVENT_HDMI_CONNECT_STATUS 0x01 // Display 1 status change
#define T6INT_DISP_EVENT_VGA_CONNECT_STATUS 0x02  // Display 2 status change
#define T6INT_DISP_EVENT_FENCE_ID 0x04
#define T6INT_DISP_EVENT_JPEG_DECODER_ERROR 0x08

// T6INT_ROM_EVENT_XXX
#define T6INT_ROM_EVENT_FINISH 0x00
#define T6INT_ROM_EVENT_CHECKSUM 0x01
#define T6INT_ROM_EVENT_ERASING 0x02
#define T6INT_ROM_EVENT_WRITING 0x03
#define T6INT_ROM_EVENT_VERIFYING 0x04
#define T6INT_ROM_EVENT_TRANSFERING 0x11 // software used.
#define T6INT_ROM_EVENT_ERROR 0x80
#define T6INT_ROM_EVENT_ERR_CHECKSUM (T6INT_ROM_EVENT_ERROR | T6INT_ROM_EVENT_CHECKSUM)
#define T6INT_ROM_EVENT_ERR_ERASE (T6INT_ROM_EVENT_ERROR | T6INT_ROM_EVENT_ERASING)
#define T6INT_ROM_EVENT_ERR_WRITE (T6INT_ROM_EVENT_ERROR | T6INT_ROM_EVENT_WRITING)
#define T6INT_ROM_EVENT_ERR_VERIFY (T6INT_ROM_EVENT_ERROR | T6INT_ROM_EVENT_VERIFYING)
#define T6INT_ROM_EVENT_ERR_TRANSFER (T6INT_ROM_EVENT_ERROR | T6INT_ROM_EVENT_TRANSFERING) // software used.

#pragma pack(push, 1)

typedef struct tT6InterruptData
{
    UINT8 FuncMask;      // 00h Function Mask. see T6INT_FUNC_MASK_XXX.
    UINT8 Rsvd_01_03[3]; // 01h Reserved.

    // cpu
    UINT8 CPUData[8]; // 04h cpu data

    // display
    UINT32 DisplayData;   // 0Ch display data.
    UINT8 DisplayRsvd[3]; // 10h display reserved.
    UINT8 DisplayEvent;   // 13h see T6INT_DISP_EVENT_XXX.

    // network
    UINT8 NetworkData[8]; // 14h network data.

    // audio
    UINT8 AudioRsvd[4]; // 1Ch audio reserved.
    union
    {
        struct
        {
            UINT32 AudioRenderDmaPtr : 24;  // audio render DMA pointer.
            UINT32 AudioRenderJack0 : 1;    // audio render jack-0 state. 0:unplug;  1:plug.
            UINT32 AudioRenderDVOH : 1;     // audio render DVO HDMI state. 0:DVI;  1:HDMI.
            UINT32 AudioRenderLVDSH : 1;    // audio render LVDS HDMI state. 0:DVI;  1:HDMI.
            UINT32 AudioRsvd : 3;           // audio reserved.
            UINT32 AudioCaptureJackMic : 1; // audio capture mic jack state. 0:unplug;  1:plug.
            UINT32 AudioCaptureDataIn : 1;  // audio capture data in.
        };
        UINT32 AudioRenderValue;
    } AudioRender; // 20h audio render.

    // serial
    UINT8 SerialData[8]; // 24h serial data?

    // rom
    UINT8 RomEvent;        // 2Ch rom event. see T6INT_ROM_EVENT_XXX.
    UINT8 RomRsvd;         // 2Dh rom reserved.
    UINT16 RomFenceId;     // 2Eh rom fence id.
    UINT32 RomProceedSize; // 30h rom proceed size.

    // others
    UINT8 Rsvd_34_3F[12]; // 34h reserved.
} T6INTERRUPTDATA, *PT6INTERRUPTDATA;

#pragma pack(pop)

#endif
