#include <stdio.h>
#include <pic18f2550.h>
#include "UART_Common.h"
#include "MatrixDisplay.h"
#include "BitByte.h"
#include "RGB.h"
#include "usb.h"

extern extern char EarRandom;

short TimerSeconds = 0;
long Seconds = 0;
short TimeoutSeconds = 0;
unsigned char AllowDisplayCommunication = 1;
unsigned char PhoneConnected = 0;
static volatile unsigned char InterruptServicesEnabled = 0;
#ifdef USB_STATE_DIAGNOSTIC
static unsigned char diagnosticTimerTicks;
static unsigned char diagnosticSlot;
static unsigned char diagnosticDisplayedRank;
extern volatile uint8_t USBStateDiagnosticHighest;
#endif

static void timer0_init(void)
{
    INTCON2bits.TMR0IP = 1;

    T0CONbits.TMR0ON = 0;
	T0CONbits.T08BIT = 0;
	T0CONbits.T0CS = 0;
	T0CONbits.T0SE = 0;
	T0CONbits.PSA = 0;
	T0CONbits.T0PS0 = 0;
	T0CONbits.T0PS1 = 1;
	T0CONbits.T0PS2 = 0;

    TMR0H = 0x66;
    TMR0L = 0x66;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    T0CONbits.TMR0ON = 1;
}

void clock_indicator_init(void)
{
    timer0_init();
    INTCONbits.GIE = 1;
}

void interrupt_init(void)
{
    PIE1bits.TXIE = 1;
    PIE1bits.RCIE = 1;
    INTCONbits.PEIE = 1;
    InterruptServicesEnabled = 1;
    timer0_init();
    INTCONbits.GIE = 1;
}

void __interrupt() tc_int(void)        // interrupt function 
{
#ifdef USB_STATE_DIAGNOSTIC
    if(INTCONbits.T0IF)
    {
        unsigned char pulseCount;

        INTCONbits.T0IF = 0;
        TMR0H = 0x66;
        TMR0L = 0x66;

        if(USBStateDiagnosticHighest != diagnosticDisplayedRank)
        {
            diagnosticDisplayedRank = USBStateDiagnosticHighest;
            diagnosticTimerTicks = 0;
            diagnosticSlot = 0;
        }

        if(USBStateDiagnosticHighest == 9)
        {
            LATCbits.LATC1 = 1;
            return;
        }

        diagnosticTimerTicks++;
        if(diagnosticTimerTicks >= 4)
        {
            diagnosticTimerTicks = 0;
            diagnosticSlot++;
            if(diagnosticSlot >= 24)
            {
                diagnosticSlot = 0;
            }
        }

        switch(USBStateDiagnosticHighest)
        {
            case 1:
                pulseCount = 1;
                break;
            case 2:
                pulseCount = 2;
                break;
            case 3:
                pulseCount = 3;
                break;
            case 4:
                pulseCount = 4;
                break;
            case 5:
                pulseCount = 5;
                break;
            case 6:
                pulseCount = 6;
                break;
            case 7:
                pulseCount = 7;
                break;
            case 8:
                pulseCount = 8;
                break;
            case 9:
                pulseCount = 9;
                break;
            default:
                pulseCount = 0;
                break;
        }

        LATCbits.LATC1 = (diagnosticSlot < pulseCount * 2) &&
                         ((diagnosticSlot & 1) == 0);
    }
#else
    if(INTCONbits.T0IF) 
    {                                     // if timer flag is set & interrupt enabled
        INTCONbits.T0IF = 0;                  // clear the interrupt flag
        TMR0H = 0x66;
        TMR0L = 0x66;
        TimerSeconds++;
        if(TimerSeconds == 38)
        {
            if(PhoneConnected > 0)
            {
                PhoneConnected--;
            }
            TimerSeconds = 0;
            Seconds++;
#ifdef CLOCK_TEST
            LATCbits.LATC1 = !LATCbits.LATC1;
#endif
            if(InterruptServicesEnabled && EarRandom)
            {
                SetEarRandom();
            }
        }
        if(InterruptServicesEnabled &&
           USBGetDeviceState() >= CONFIGURED_STATE)
        {
            if(AllowDisplayCommunication == 1)
            {
                UpdateDisplay();
            }
            else
            {
                SetRGB(BLACK);
            }

            UpdateStatusLed();
        }
        
        if(InterruptServicesEnabled && TimeoutSeconds > 0)
        {
            TimeoutSeconds--;
            if(TimeoutSeconds == 0)
            {
                PuntRxCom = 0;
                TotRxCom = 0;
            }
        }
    }
	
	if(InterruptServicesEnabled && PIR1bits.TXIF==1) //Polling for transmission interrupt
    {
        if(PuntTxCom >= TotTxCom)
        {
            PIE1bits.TXIE = 0;  //TXIE
            PuntTxCom = 0;
            TotTxCom = 0;
        }
        else
        {
            TXREG=BuffTxCom[PuntTxCom];
            PuntTxCom++;
        }
    }
	
	if(InterruptServicesEnabled && PIR1bits.RCIF==1) //Polling for reception interrupt
    {
        if(PuntRxCom < BYTE_BUFFER_COM)
        {
            BuffRxCom[PuntRxCom] = RCREG;
            PuntRxCom++;
			if(PuntRxCom==1)
			{
				TotRxCom = (unsigned char)(BuffRxCom[0]+1);
			}
        }

        if(TotRxCom == PuntRxCom)
        {
            PuntRxCom = 0;
            TotRxCom = 0;
            BancataRicevuta = 1;
        }
        TimeoutSeconds = 190; //e seconds
    } 
#endif
}
