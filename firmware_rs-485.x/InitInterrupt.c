#include <stdio.h>
#include <pic18f2550.h>
#include "UART_Common.h"
#include "MatrixDisplay.h"
#include "BitByte.h"
#include "RGB.h"

extern extern char EarRandom;

void interrupt_init()
{
    PIE1bits.TXIE = 1;
    PIE1bits.RCIE = 1;
	
    INTCONbits.PEIE = 1;
    
    INTCON2bits.TMR0IP = 1;
    
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0; 
    
    
    T0CONbits.TMR0ON = 1;
	T0CONbits.T08BIT = 0;
	T0CONbits.T0CS = 0;
	T0CONbits.T0SE = 0;
	T0CONbits.PSA = 0;
	T0CONbits.T0PS0 = 0;
	T0CONbits.T0PS1 = 0;
	T0CONbits.T0PS2 = 0;
    
    INTCONbits.GIE = 1;
}

short TimerSeconds = 0;
long Seconds = 0;
short TimeoutSeconds = 0;
unsigned char AllowDisplayCommunication = 0;
unsigned char PhoneConnected = 0;

void __interrupt() tc_int(void)        // interrupt function 
{
    if(INTCONbits.T0IF) 
    {                                     // if timer flag is set & interrupt enabled
        INTCONbits.T0IF = 0;                  // clear the interrupt flag
        TimerSeconds++;
        if(TimerSeconds == 38)
        {
            if(PhoneConnected > 0)
            {
                PhoneConnected--;
            }
            TimerSeconds = 0;
            Seconds++;
            if(EarRandom)
            {
                SetEarRandom();
            }
        }
        if(AllowDisplayCommunication == 1)
        {
            UpdateDisplay();
            UpdateStatusLed();
        }
        else
        {
            ClearScreen();
            SetRGB(BLACK);
            Refresh();
            UpdateStatusLed();
        }
        
        if(TimeoutSeconds > 0)
        {
            TimeoutSeconds--;
            if(TimeoutSeconds == 0)
            {
                PuntRxCom = 0;
                TotRxCom = 0;
            }
        }
    }
	
	if(PIR1bits.TXIF==1)                     //Polling for transmission interrupt
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
	
	if(PIR1bits.RCIF==1)                     //Polling for reception interrupt
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
	
}

