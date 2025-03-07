#include "common.h"



extern unsigned char BuffTxCom[BYTE_BUFFER_COM];
extern unsigned char BuffRxCom[BYTE_BUFFER_COM];

extern unsigned char PuntTxCom;
extern unsigned char PuntRxCom;

extern unsigned char TotTxCom;
extern unsigned char TotRxCom;

extern unsigned char BancataRicevuta;


extern char UART_Init(long int baudrate);
extern char UART_Data_Ready();
extern char UART_Read();
extern void UART_Write(char data);


#define ricOK      0x00

#define TotBancate  ricOK + 1


unsigned char Bancate[TotBancate][BYTE_BUFFER_COM];


