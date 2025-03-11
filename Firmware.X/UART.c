#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "RGB.h"
#include "common.h"


char UART_Init()
{
  long int baudrate = 9600;
  unsigned int x;
  x = (_XTAL_FREQ - baudrate*64)/(baudrate*64);   //SPBRG for Low Baud Rate
  if(x>255)                                       //If High Baud Rage Required
  {
    x = (_XTAL_FREQ - baudrate*16)/(baudrate*16); //SPBRG for High Baud Rate
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


char UART_Data_Ready()
{
  return RCIF;
}
char UART_Read()
{
  while(!RCIF);
  return RCREG;
}



void UART_Write(char data)
{
  while(!TRMT);
  TXREG = data;
}

void lock_RGB()
{
counter=0;
limit=100000;
color = color;
}


void UART_Write_Text(char *text)
{
  int i;
  for(i=0;text[i]!='\0';i++)
    UART_Write(text[i]);
}
int result = 0;
char parola[20];//test
char nome[20];//test
void CheckChangeRGB()
{
    memset(parola,0x00,20);//test
    memset(nome,0x00,20);//test
    if(UART_Data_Ready())
    {
        result = UART_Read();
        switch(result)
        {
            case 0x30: SetRGB(BLACK); lock_RGB(); break;
            case 0x31: SetRGB(GREEN); lock_RGB(); break;
            case 0x32: SetRGB(BLUE); lock_RGB(); break;
            case 0x33: SetRGB(RED); lock_RGB(); break;
            case 0x34: SetRGB(VIOLET); lock_RGB(); break;
            case 0x35: SetRGB(YELLOW); lock_RGB(); break;
            case 0x36: SetRGB(CIAN); lock_RGB(); break;
            case 0x37: SetRGB(WHITE); lock_RGB(); break;
            case 0x38: SetLed(1); break;
            case 0x39: SetLed(0); break;
            case '[' ://test
                SetLed(1);
                while(!UART_Data_Ready())
                {  
                    result = UART_Read();
                    nome[nome[19]] = result;
                    if((result == ']') || (nome[19] == 18))
                    {
                        nome[nome[19]] = 0x00;
                        break;
                    }
                    nome[19]++;
                }
                sprintf(parola,"Ciao %s!",nome);
                UART_Write_Text((char*)parola);
                SetLed(0);
                break;
        }
    }
}