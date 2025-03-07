/*
 * File:   RGB.c
 * Author: Marco
 *
 * Created on 20 marzo 2018, 20.54
 */


#include <xc.h>
#include <stdlib.h>
#include "Interrupt.h"
#include "common.h"

unsigned char G,B,R,Blink;
unsigned char EarRandom = 0;

void SetRGB_Blink(unsigned char value)
{
    Blink = value;
}
void SetRGB(unsigned char Color)
{
    EarRandom = 0;
    R = (unsigned char)((Color>>2)&0x01);
    B = (unsigned char)((Color>>1)&0x01);
    G = (unsigned char)(Color&0x01);
}

int tempCounter=0;
unsigned char changeStatus = 0;
void UpdateStatusLed()
{
    if(Blink==BLINK_NO)
    {
        LATAbits.LATA3 = (unsigned char)(!G);
        LATAbits.LATA4 = (unsigned char)(!B);
        LATAbits.LATA5 = (unsigned char)(!R);
        
    }
    else
    {
        tempCounter++;
        if(tempCounter >= Blink)
        {
            tempCounter = 0;
            if(changeStatus == 0)
            {
                        changeStatus = 1;
                        LATAbits.LATA3 = (unsigned char)(!G);
                        LATAbits.LATA4 = (unsigned char)(!B);
                        LATAbits.LATA5 = (unsigned char)(!R);
            }
            else
            {
                        changeStatus = 0;
                        LATAbits.LATA3 = 1;
                        LATAbits.LATA4 = 1;
                        LATAbits.LATA5 = 1;
            }
        }
    }
}

void SetLed(char value)
{
    LATCbits.LATC1 = (unsigned char)(value&0x01);  //red led mainboard on
}

unsigned char precvalue = 0;
void SetEarRandom()
{
    unsigned char actualvalue;
    actualvalue = rand() % 7;
    actualvalue++;
    if(actualvalue == precvalue)
    {
        if(actualvalue==7)
            actualvalue = 1;
        else
            actualvalue++;
    }
    SetRGB(actualvalue);
    EarRandom = 1;
    precvalue = actualvalue;
}
