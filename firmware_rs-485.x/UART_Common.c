#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "BitByte.h"
#include "common.h"
#include "RGB.h"


unsigned char BuffTxCom[BYTE_BUFFER_COM];
unsigned char BuffRxCom[BYTE_BUFFER_COM];

unsigned char PuntTxCom = 0;
unsigned char PuntRxCom = 0;

unsigned char TotTxCom = 0;
unsigned char TotRxCom = 0;

unsigned char BancataRicevuta = 0;




char UART_Init(long int baudrate)
{
  unsigned int x;
  x = (unsigned int)((_XTAL_FREQ - baudrate*64)/(baudrate*64));   //SPBRG for Low Baud Rate
  if(x>255)                                       //If High Baud Rage Required
  {
    x = (unsigned int)((_XTAL_FREQ - baudrate*16)/(baudrate*16)); //SPBRG for High Baud Rate
    BRGH = 1;                                     //Setting High Baud Rate
  }
  
  if(x<256)
  {
    SPBRG = x;                                    //Writing SPBRG Register
    SYNC = 0;                                     //Setting Asynchronous Mode, ie UART
    SPEN = 1;                                     //Enables Serial Port
    TRISC7 = 1;                                   //As Prescribed in Datasheet
    TRISC6 = 1;                                   //As Prescribed in Datasheet
    CREN = 1;                                     //Enables Continuous Reception
    TXEN = 1;                                     //Enables Transmission
    return 1;                                     //Returns 1 to indicate Successful Completion
  }
  
  return 0;                                       //Returns 0 to indicate UART initialization failed
}
