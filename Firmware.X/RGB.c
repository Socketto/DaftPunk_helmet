/*
 * File:   RGB.c
 * Author: Marco
 *
 * Created on 20 marzo 2018, 20.54
 */


#include <xc.h>

void SetRGB(unsigned Color)
{
    LATAbits.LATA3 = Color;
    LATAbits.LATA4 = Color>>1;
    LATAbits.LATA5 = Color>>2;
    LATAbits.LATA3 = !LATAbits.LATA3;
    LATAbits.LATA4 = !LATAbits.LATA4;
    LATAbits.LATA5 = !LATAbits.LATA5;
}

void SetLed(char value)
{
    LATCbits.LATC1 = value&0x01;  //red led mainboard on
}