#include "common.h"
#include <xc.h>
#include <pic18f2550.h>

//https://electrosome.com/i2c-pic-microcontroller-mplab-xc8/

#define I2C_WRITE_RETRY      3

const unsigned long I2C_Baudrate = 100000;

void i2c_init(void)
{
    TRISB |= 0x03;                              //Configuring SDA and SCL as input pins
    SSPCON1 = 0x28;                             //Configuring as I2C Master mode with SCL clock frequency= FOSC/(4 * (SSPADD + 1))
    SSPADD = (_XTAL_FREQ/(4*I2C_Baudrate))-1;              //Setting SCL frequency with value in clock_freq ??(100k 0x31)??
    SSPSTAT = 0x00;                             //Resetting SSPSTAT register
    SSPIF = 0; 
}

void mssp_wait(void)
{
    while(!SSPIF);                              //wait till SSPIF flag is set
    SSPIF=0;
}

void i2c_restart(void)
{
    RSEN=1;                                     //Initiate restart condition
    mssp_wait();                                //Wait till completion of event
}

void i2c_idle(void)
{
  // Wait for Status Idle (i.e. ACKEN, RCEN, PEN, RSEN, SEN) and I2C Bus stop detection
  while (( SSPCON2 & 0x1F ) || ( SSPSTAT & 0x04 ));
}
 
 
void i2c_start(void)
{
	SSPCON1 = 0x28;
    i2c_idle();                                 //Check if the I2C bus is idle
    SEN=1;                                      //Initiate start condition
    mssp_wait();                                //Wait till completion of event
}

unsigned char i2c_write(unsigned char data)
{
	unsigned char i2c_Retry = I2C_WRITE_RETRY;
	SSPCON1 = 0x28;
    L1: SSPBUF=data;                            //Input data to buffer
    mssp_wait();	//Wait till completion of event
	i2c_Retry--;
	if(i2c_Retry == 0)
		return 0;
    if(ACKSTAT)                                 //check if acknowledgement signal received 
    {
        i2c_restart();                          //If no acknowledgement received, give restart signal and transmit data again
        goto L1;
    }
	return 1;
}

unsigned char i2c_read( unsigned char ack )
{
    RCEN=1;                                     //Enable reception in Master device 
    while(!BF);                                 //Wait till buffer is full
    RCEN=0;                                     //Enable reception in Master device 
    
    if ( ack )
    {
        ACKDT=0;
    }
    else
    {
        ACKDT=1;
    }
    ACKEN=1;                                    //Send acknowledge sequence
    
    return SSPBUF;                              //return received data
    
}
/*
void i2c_ack(void)
{
    ACKDT=0;                                    //Set as acknowledgement
    ACKEN=1;                                    //Initiate acknowledgement signal
    mssp_wait();                                //Wait till completion of event
}
 
void i2c_nack(void)
{
    ACKDT=1;                                    //Set as negative acknowledgement
    ACKEN=1;                                    //Initiate negative acknowledgement signal
    mssp_wait();                                //Wait till completion of event
}
*/
void i2c_stop(void)
{
	i2c_idle();
    PEN=1;                                      //Initiate Stop condition
    while(PEN) {};
    mssp_wait();                                //Wait till completion of event
    SSPEN=0;                                    //Disable I2C operation
}

