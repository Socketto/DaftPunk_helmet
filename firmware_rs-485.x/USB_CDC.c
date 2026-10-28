#include <string.h>

#include "common.h"
#include "USB_CDC.h"
#include "usb.h"
#include "usb_device_cdc.h"

#define USB_STREAM_BUFFER_SIZE (2 * CDC_DATA_OUT_EP_SIZE)

const USB_DEVICE_DESCRIPTOR device_dsc =
{
    sizeof(USB_DEVICE_DESCRIPTOR),
    USB_DESCRIPTOR_DEVICE,
    0x0200,
    0x02,
    0x00,
    0x00,
    64,
    0x04D8,
    0x000A,
    0x0100,
    0,
    0,
    0,
    1
};

static const unsigned char configurationDescriptor[] =
{
    9, USB_DESCRIPTOR_CONFIGURATION, 67, 0, 2, 1, 0, 0x80, 50,
    9, USB_DESCRIPTOR_INTERFACE, 0, 0, 1, COMM_INTF, ABSTRACT_CONTROL_MODEL, V25TER, 0,
    5, CS_INTERFACE, DSC_FN_HEADER, 0x10, 0x01,
    5, CS_INTERFACE, DSC_FN_CALL_MGT, 0, 1,
    4, CS_INTERFACE, DSC_FN_ACM, USB_CDC_ACM_FN_DSC_VAL,
    5, CS_INTERFACE, DSC_FN_UNION, 0, 1,
    7, USB_DESCRIPTOR_ENDPOINT, 0x81, 0x03, CDC_COMM_IN_EP_SIZE, 0, 16,
    9, USB_DESCRIPTOR_INTERFACE, 1, 0, 2, DATA_INTF, 0, NO_PROTOCOL, 0,
    7, USB_DESCRIPTOR_ENDPOINT, 0x02, 0x02, CDC_DATA_OUT_EP_SIZE, 0, 0,
    7, USB_DESCRIPTOR_ENDPOINT, 0x82, 0x02, CDC_DATA_IN_EP_SIZE, 0, 0
};

static const unsigned char languageDescriptor[] =
{
    4, USB_DESCRIPTOR_STRING, 0x09, 0x04
};

const unsigned char *const USB_CD_Ptr[] =
{
    configurationDescriptor
};

const unsigned char *const USB_SD_Ptr[] =
{
    languageDescriptor
};

static unsigned char streamBuffer[USB_STREAM_BUFFER_SIZE];
static unsigned char streamHead;
static unsigned char streamTail;
static unsigned char streamCount;
static unsigned char receiveFrame[BYTE_BUFFER_COM];
static unsigned char receiveFrameLength;
static unsigned char expectedFrameLength;
static unsigned char readyFrameLength;
static unsigned char frameReady;
static unsigned char transmitPending;
static unsigned char transmitBuffer[BYTE_BUFFER_COM];
static unsigned char transmitLength;
static unsigned char packetBuffer[CDC_DATA_OUT_EP_SIZE];

bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch((uint16_t)event)
    {
        case EVENT_CONFIGURED:
            CDCInitEP();
            break;
        case EVENT_EP0_REQUEST:
            USBCheckCDCRequest();
            break;
        default:
            break;
    }

    return true;
}

static void ResetCommunicationState(void)
{
    streamHead = 0;
    streamTail = 0;
    streamCount = 0;
    receiveFrameLength = 0;
    expectedFrameLength = 0;
    readyFrameLength = 0;
    frameReady = 0;
    transmitPending = 0;
}

static void QueueReceivedBytes(const unsigned char *data, unsigned char length)
{
    unsigned char index;

    for(index = 0; index < length; index++)
    {
        streamBuffer[streamTail] = data[index];
        streamTail++;
        if(streamTail == USB_STREAM_BUFFER_SIZE)
        {
            streamTail = 0;
        }
        streamCount++;
    }
}

static unsigned char DequeueReceivedByte(void)
{
    unsigned char value;

    value = streamBuffer[streamHead];
    streamHead++;
    if(streamHead == USB_STREAM_BUFFER_SIZE)
    {
        streamHead = 0;
    }
    streamCount--;
    return value;
}

static void ParseReceivedBytes(void)
{
    unsigned char value;

    while(streamCount > 0 && frameReady == 0)
    {
        value = DequeueReceivedByte();
        if(receiveFrameLength == 0)
        {
            expectedFrameLength = (unsigned char)(value + 1);
            if(expectedFrameLength < 2 || expectedFrameLength > BYTE_BUFFER_COM)
            {
                expectedFrameLength = 0;
                continue;
            }

            receiveFrame[0] = value;
            receiveFrameLength = 1;
        }
        else
        {
            receiveFrame[receiveFrameLength] = value;
            receiveFrameLength++;
            if(receiveFrameLength == expectedFrameLength)
            {
                frameReady = 1;
                readyFrameLength = receiveFrameLength;
                receiveFrameLength = 0;
                expectedFrameLength = 0;
            }
        }
    }
}

void USB_CDC_Init(void)
{
    ResetCommunicationState();
    USBDeviceInit();
    USBDeviceAttach();
}

void USB_CDC_Task(void)
{
    unsigned char receivedLength;

    USBDeviceTasks();

    if(USBGetDeviceState() < CONFIGURED_STATE)
    {
        ResetCommunicationState();
        return;
    }

    if(USBIsDeviceSuspended())
    {
        return;
    }

    if(transmitPending != 0 && USBUSARTIsTxTrfReady())
    {
        putUSBUSART(transmitBuffer, transmitLength);
        transmitPending = 0;
    }

    CDCTxService();
    ParseReceivedBytes();
    if(frameReady != 0)
    {
        return;
    }

    if(transmitPending == 0 &&
       (USB_STREAM_BUFFER_SIZE - streamCount) >= CDC_DATA_OUT_EP_SIZE)
    {
        receivedLength = getsUSBUSART(packetBuffer, sizeof(packetBuffer));
        if(receivedLength > 0)
        {
            QueueReceivedBytes(packetBuffer, receivedLength);
            ParseReceivedBytes();
        }
    }

    CDCTxService();
}

unsigned char USB_CDC_FrameReady(void)
{
    return (unsigned char)(frameReady != 0 && transmitPending == 0);
}

void USB_CDC_GetFrame(unsigned char *frame)
{
    if(USB_CDC_FrameReady())
    {
        memset(frame, 0, BYTE_BUFFER_COM);
        memcpy(frame, receiveFrame, readyFrameLength);
        frameReady = 0;
        readyFrameLength = 0;
        ParseReceivedBytes();
    }
}

unsigned char USB_CDC_Send(const unsigned char *data, unsigned char length)
{
    if(length == 0 || length > BYTE_BUFFER_COM || transmitPending != 0)
    {
        return 0;
    }

    memcpy(transmitBuffer, data, length);
    transmitLength = length;
    transmitPending = 1;
    return 1;
}
