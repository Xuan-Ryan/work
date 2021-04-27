
#ifndef _T6USBDONGLE_H_
#define _T6USBDONGLE_H_

#include "t6.h"
#include "t6auddef.h"

typedef struct tMCT_USBCTRL_CMD
{
  int bmRequestType;
  int bRequest;
  int wValue;
  int wIndex;
  char bytes[2048];
  int size;
  int timeout;
} __attribute__((packed)) MCT_USBCTRL_CMD, *PMCT_USBCTRL_CMD;

typedef struct tSEND_MCT_USBCTRL_CMD
{
  int bmRequestType;
  int bRequest;
  int wValue;
  int wIndex;
  int size;
  int timeout;
  char bytes[1024];
} __attribute__((packed)) SEND_MCT_USBCTRL_CMD, *PSEND_MCT_USBCTRL_CMD;

#define IOC_MAGIC 'MCT'
#define IOCTL_SENDCMD _IOWR(IOC_MAGIC, 1, MCT_USBCTRL_CMD)
#define IOCTL_SENDCMD2 _IOWR(IOC_MAGIC, 2, SEND_MCT_USBCTRL_CMD)

void Dump_BulkCmdHdr(PBULK_CMD_HEADER bulkcmdhdr);
void Dump_VideoFlipHdr(PVIDEO_FLIP_HEADER vfliphdr);
void Dump_CtrlVenderHdr(PCTRL_VENDOR_CMD ctrlhdr);
void Dump_IntData(char *data);

int myusb_control_msg(int fd, int bmRequestType, int bRequest, int wValue, int wIndex, char *bytes, int size, int timeout);
int myusb_control_msg_send(int fd, int bmRequestType, int bRequest, int wValue, int wIndex, char *bytes, int size, int timeout);

int myusb_bulk_write(int fd, char *buf, int size);
int myusb_interrupt_read(int fd, char *buf, int size);

int usbdongle_open();
int usbdongle_open_s(char *phander);

int t6_set_resolution(int fd, int w, int h);
void t6donglereset(int fd);
int t6_dongle_close(int fd);
void t6_set_displayinterface(int inf);

int t6_get_monitorstatus(int fd, char view); // veiw = 0 hdmi , view =1 vga;
int t6_set_video_data(int fd, char *data, int fbaddr, int cmdaddr);
int t6_SendAudio(int fd, char *data, int len);
int t6_send_rab_data(int fd, int Width, int Height, UINT8 *pbuf, int datasize, int fbaddr);
int t6_get_edid(int fd, unsigned char *EdidBuf);

void initipcsharememory(void);
int t6_set_disp_preview(int fd, int dw, int dh, int x, int y, int fw, int fh, int cmdaddr, int flag, char *data, int TotalLength);

int t6_set_disp_active(int fd, int w, int h, char *data, int len);
int t6_clear_block(int fd, int x, int y, int w, int h);
int t6_clear_all(int fd);
//int t6_video_pass(int fd ,char* image_data,int TotalLength,int flag);

//int t6_video_pass(int fd ,char* image_data,int TotalLength ,int flag );
void t6_cmd_reorganization(char *image_data, int *last_w, int *last_h);
int t6_video_pass(int fd, char *image_data, int TotalLength, int x, int y, int w, int h, int flag);

int t6_write_rom_file(int fd, char *buf, int len);

int t6_set_ssid1(int fd, char *str, int len);
int t6_set_ssid2(int fd, char *str, int len);
int t6_set_pw1(int fd, char *str, int len);
int t6_set_pw2(int fd, char *str, int len);
int t6_set_gssid1(int fd, char *str, int len);
int t6_set_gssid2(int fd, char *str, int len);
int t6_set_gpw1(int fd, char *str, int len);
int t6_set_gpw2(int fd, char *str, int len);

int t6_test_mode(int fd);
int t6_set_msg(int fd, int x, int y, int w, int h, char on);

#endif
