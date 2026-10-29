/*
 * File:   Main.c
 * Author: Marco
 *
 * Created on 19 marzo 2018, 22.06
 */
// PIC18F2550 Configuration Bit Settings

// 'C' source line config statements
  
// CONFIG1L
#pragma config PLLDIV = 5       // PLL Prescaler Selection bits (20 MHz oscillator input divided by 5)
#pragma config CPUDIV = OSC1_PLL2// System Clock Postscaler Selection bits ([Primary Oscillator Src: /1][96 MHz PLL Src: /2])
#pragma config USBDIV = 2       // USB clock is 48 MHz (96 MHz PLL divided by 2)

// CONFIG1H
#pragma config FOSC = HSPLL_HS  // Oscillator Selection bits (HS oscillator, PLL enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOR = ON         // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown-out Reset Voltage bits (Minimum setting 2.05V)
#pragma config VREGEN = ON      // USB Voltage Regulator Enable bit (USB voltage regulator enabled)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = ON      // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer 1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) is not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) is not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) is not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) is not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) is not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM is not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) is not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) is not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) is not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) is not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) are not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) is not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM is not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) is not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) is not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h>

#include <pic18f2550.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "RGB.h"
#include "UART_Common.h"
#include "I2C.h"
#include "Interrupt.h"
#include "MCP23017.h"
#include "MatrixDisplay.h"
#include "24lc256.h"
#include "USB_CDC.h"

extern unsigned char CurrentExecution;
extern char EarRandom;
unsigned char Frase[100];
unsigned char tempUnsignedChar;
unsigned char EEPROM = 0;
unsigned short ShortTemp;
void Init()
{
    unsigned int startupDelay;

    TRISA = 0b00000000;//all output
    TRISB = 0b00000011;//all output
    TRISC = 0b10000000;//PortC7 input RX
    TRISC |= 0b00110000;

#ifdef USB_STATE_DIAGNOSTIC
    TRISCbits.TRISC1 = 0;
    LATCbits.LATC1 = 0;
    clock_indicator_init();
    for(startupDelay = 0; startupDelay < 500; startupDelay++)
    {
        __delay_ms(1);
    }
    USB_CDC_Init();
#else
#ifdef CLOCK_TEST
    TRISCbits.TRISC1 = 0;
    LATCbits.LATC1 = 0;
    clock_indicator_init();
#endif

    for(startupDelay = 0; startupDelay < 500; startupDelay++)
    {
        __delay_ms(1);
    }

    USB_CDC_Init();

    for(startupDelay = 0;
        startupDelay < 5000 && !USB_CDC_IsConfigured();
        startupDelay++)
    {
        USB_CDC_Task();
        __delay_ms(1);
    }

#if HC06
    UART_Init(9600);
#else
    UART_Init(19200);
    LATCbits.LATC2 = 0; //for RS-485
#endif

    i2c_init();
    EEPROM = EE_Init();

    for(tempUnsignedChar = 0 ; tempUnsignedChar < MCP_COUNTS ; tempUnsignedChar++)
	{
        Init_MCP(MCPAddresses[tempUnsignedChar]);
        USB_CDC_Task();
    }
    
    interrupt_init();

    //SetRGB_Blink(BLINK_NO);
    //SetRGB(BLACK);
	SetEarRandom();
	SetRGB_Blink(BLINK_500_mSecond);
#endif
}

void SendByte()
{
    PuntTxCom = 0;
    PuntTxCom++;
    TXREG=BuffTxCom[0];
    PIE1bits.TXIE = 1;  //TXIE
}

unsigned int tempp;
unsigned short counterpollo = 0;

/*
void test()
{
    for(tempp = 0;tempp<379;tempp++)
    {
          ClearScreen();
          Write_Column(0,(unsigned char)EE_byte_read(0+tempp));
          Write_Column(1,(unsigned char)EE_byte_read(1+tempp));
          Write_Column(2,(unsigned char)EE_byte_read(2+tempp));
          Write_Column(3,(unsigned char)EE_byte_read(3+tempp));
          Write_Column(4,(unsigned char)EE_byte_read(4+tempp));
          Write_Column(5,(unsigned char)EE_byte_read(5+tempp));
          Write_Column(6,(unsigned char)EE_byte_read(6+tempp));
          Write_Column(7,(unsigned char)EE_byte_read(7+tempp));
          Write_Column(8,(unsigned char)EE_byte_read(8+tempp));
          Write_Column(9,(unsigned char)EE_byte_read(9+tempp));
          Write_Column(10,(unsigned char)EE_byte_read(10+tempp));
          Write_Column(11,(unsigned char)EE_byte_read(11+tempp));
          Write_Column(12,(unsigned char)EE_byte_read(12+tempp));
          Write_Column(13,(unsigned char)EE_byte_read(13+tempp));
          Write_Column(14,(unsigned char)EE_byte_read(14+tempp));
          Write_Column(15,(unsigned char)EE_byte_read(15+tempp));
          Refresh();
          __delay_ms(200);
    }
}*/

unsigned char MusicValue;
extern unsigned char PhoneConnected;
unsigned char SignedError = 0;
extern unsigned char AllowDisplayCommunication;
#ifdef USB_STATE_DIAGNOSTIC
void main(void)
{
    Init();
    while(1)
    {
        USB_CDC_Task();
    }
}
#else
void main(void) {
    unsigned char commandFrame[BYTE_BUFFER_COM];
    unsigned char commandResponse[BYTE_BUFFER_COM];
    unsigned char commandFromUsb;
    unsigned char responseLength;

    Init();
    
    /**/
    //test();
    /**/
    ClearScreen();
    Refresh();
    while(1)
    {
        USB_CDC_Task();

        if(RCSTAbits.OERR)
        {
            RCSTAbits.CREN = 0;
            SignedError = 100;
        }
        if(RCSTAbits.FERR)
        {
            tempp = RCREG;
            SignedError = 100;
        }
        
#if !defined(CLOCK_TEST) && !defined(USB_STATE_DIAGNOSTIC)
        if(SignedError>0)
        {
            SignedError--;
            SetLed(1);
        }
        else
        {
            SetLed(0);
        }
#endif
        
		if(BancataRicevuta || USB_CDC_FrameReady())
		{
            commandFromUsb = 0;
            if(BancataRicevuta)
            {
                BancataRicevuta = 0;
                memcpy(commandFrame, BuffRxCom, BYTE_BUFFER_COM);
            }
            else
            {
                commandFromUsb = 1;
                USB_CDC_GetFrame(commandFrame);
            }

            commandResponse[0] = 0x01;
            commandResponse[1] = 0x0A;
			switch(commandFrame[1])
			{
				case 0x00:
					AllowDisplayCommunication = 1;
                    SetRGB(BLACK);
				break;
				case 0x01:
					AllowDisplayCommunication = 1;
                    SetRGB(RED);
				break;
				case 0x02:
					AllowDisplayCommunication = 1;
                    SetRGB(GREEN);
				break;
				case 0x03:
					AllowDisplayCommunication = 1;
                    SetRGB(BLUE);
				break;
				case 0x04:
					AllowDisplayCommunication = 1;
                    SetRGB(VIOLET);
				break;
				case 0x05:
					AllowDisplayCommunication = 1;
                    SetRGB(YELLOW);
				break;
				case 0x06:
					AllowDisplayCommunication = 1;
                    SetRGB(CIAN);
				break;
				case 0x07:
					AllowDisplayCommunication = 1;
                    SetRGB(WHITE);
				break;
                case 0x08:
					AllowDisplayCommunication = 1;
                    SetEarRandom();
                    break;
				case 0x0A:
					AllowDisplayCommunication = 1;
                    SetRGB_Blink(BLINK_NO);
				break;
				case 0x0B:
					AllowDisplayCommunication = 1;
                    SetRGB_Blink(BLINK_500_mSecond);
				break;
				case 0x0C:
					AllowDisplayCommunication = 1;
                    SetRGB_Blink(BLINK_1_Second);
				break;
				case 0x0D:
					AllowDisplayCommunication = 1;
                    SetRGB_Blink(BLINK_184_mSecond);
				break;
				case 0x0E:
					AllowDisplayCommunication = 1;
                    SetRGB_Blink(BLINK_75_mSecond);
				break;
                case 0x10:
                    switch(commandFrame[2])
                    {
                        case 0x01:
                            commandResponse[0] = 0x02;
                            ShortTemp = commandFrame[3];
                            ShortTemp *= 0x0100;
                            ShortTemp += commandFrame[4];
                            if(EE_byte_read(ShortTemp, &commandResponse[2]))
                            {
                                commandResponse[1] = 0x0A;
                            }
                            else
                            {
                                commandResponse[1] = 0x0B;
                                commandResponse[2] = 0xFF;
                            }
                            break;
                        case 0x02:
                            ShortTemp = commandFrame[3];
                            ShortTemp *= 0x0100;
                            ShortTemp += commandFrame[4];
                            commandResponse[1] = EE_byte_write(ShortTemp, commandFrame[5]) ? 0x0A : 0x0B;
                            break;
                        case 0x03:
                            AllowDisplayCommunication = 0;
                            ClearScreen();
                            SetRGB(BLACK);
                            Refresh();
                            break;
                        case 0x04:
                            AllowDisplayCommunication = 1;
                            break;
                    }
                    break;
                case 0xDC:
                    PhoneConnected = 5;
					MusicValue= commandFrame[2];
                    break;
                case 0x20:
					AllowDisplayCommunication = 1;
                    switch(commandFrame[2])
                    {
                        case 0x01:
                            commandResponse[0] = 0x02;
                            commandResponse[1] = 0x0A;
                            commandResponse[2] = ReadColumn(commandFrame[3]);
                            break;
                        case 0x02:
                            Write_Column(commandFrame[3],commandFrame[4]);
                            break;
                        case 0x03:
                            ClearScreen();
                            break;
                        case 0x04:
                            Write_Point(commandFrame[3],commandFrame[4]);
                            break;
                        case 0x05:
                            InvertScreen();
                            break;
						case 0x06:
                            Refresh();
						break;
                    }
                break;
				case 0x50:
					AllowDisplayCommunication = 1;
                    switch(commandFrame[2])
                    {
						case 0x01:
							StopScriviFrase();
							memset(Frase,0x00,100);
						    for(Frase[99] = 0 ; Frase[99] <= commandFrame[0] - 3 ; Frase[99]++)
							{
								Frase[Frase[99]] = (unsigned char)(commandFrame[(unsigned char) (2+commandFrame[2]+Frase[99])]);
							}
                            CurrentExecution = 3;
							/* to do */
							/*SendString	0xLL	0x50	0x01	0xAA	0xBB	…...		0x01	0x0A */
						break;
					}
				break;
			}
            responseLength = (unsigned char)(commandResponse[0] + 1);
            if(commandFromUsb)
            {
                if(!USB_CDC_Send(commandResponse, responseLength))
                {
                    SignedError = 100;
                }
            }
            else
            {
                memcpy(BuffTxCom, commandResponse, responseLength);
                TotTxCom = responseLength;
                SendByte();  //for send first byte!
            }
		}
		
		
#if HC06
    //CheckChangeRGB();
#else
    CheckModBus();
#endif


	//__delay_ms(10);
    }
    return;
}
#endif
