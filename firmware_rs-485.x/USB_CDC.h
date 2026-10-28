#ifndef USB_CDC_H
#define USB_CDC_H

void USB_CDC_Init(void);
void USB_CDC_Task(void);
unsigned char USB_CDC_FrameReady(void);
void USB_CDC_GetFrame(unsigned char *frame);
unsigned char USB_CDC_Send(const unsigned char *data, unsigned char length);

#endif
