// PIC18 in C - Versione 0.1a - Settembre 2014
// Copyright (c) 2014 VincenzoV.net
// Creative Commons | Attribuzione-Condividi allo stesso modo 3.0 Unported.
// Creative Commons | Attribution-Share Alike 3.0 Unported
// https://www.vincenzov.net/tutorial/PIC18/i2c-PLIB.htm

// I2C master - MCP23017

#define LED 0x02            // 00000010 - LED is ON, others is OFF

// TABLE 1-2: REGISTER ADDRESSES - Default bank 0
#define MCP23017_IODIRA 0	   // IODIRB register address
#define MCP23017_IODIRB 1	   // IODIRB register address

#define MCP23017_GPIOA 0x012   // GPIOB register address
#define MCP23017_GPIOB 0x013   // GPIOB register address


#include "I2C.h"
#include "MCP23017.h"

unsigned char MCPAddresses[MCP_COUNTS] = {0x40,0x42,0x44,0x46,0x48,0x4A,0x4C,0x4E};


void Init_MCP(unsigned char Address)
{
    unsigned char addrRW;
    i2c_start(); // Send START condition

    addrRW = Address; // 7 bit address + Write bit

    i2c_write(addrRW);

    // Configure port B as output - see MCP23017 data sheet
    i2c_write(MCP23017_IODIRB); 
    
    i2c_write(0x00); // Send Output

    i2c_stop(); // Send STOP condition
	
	
	
    i2c_start(); // Send START condition

    addrRW = Address; // 7 bit address + Write bit

    i2c_write(addrRW);

    // Configure port A as output - see MCP23017 data sheet
    i2c_write(MCP23017_IODIRA); 
    
    i2c_write(0x00); // Send Output

    i2c_stop(); // Send STOP condition
	
}



void SetOutputs(unsigned char Address,unsigned char PortA, unsigned char PortB)
{
    unsigned char addrRW;
    i2c_start(); // Send START condition

    addrRW = Address; // 7 bit address + Write bit

    i2c_write(addrRW); 

    // Put data to PortA
    i2c_write(MCP23017_GPIOA);

    i2c_write(PortA);

    i2c_stop(); // Send STOP condition
	
    i2c_start(); // Send START condition

    addrRW = Address; // 7 bit address + Write bit

    i2c_write(addrRW); 

    // Put data to PortB
    i2c_write(MCP23017_GPIOB);

    i2c_write(PortB);

    i2c_stop(); // Send STOP condition
}