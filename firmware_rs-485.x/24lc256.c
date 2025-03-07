#include "I2C.h"
#include "common.h"
#include <xc.h>
#include <pic18f2550.h>

#define I2C_EEPROM_ADDRESS  0xA0


unsigned char EE_Init()
{	
	unsigned char reponse;
    
	i2c_start();
	reponse = i2c_write(I2C_EEPROM_ADDRESS); 

	i2c_write(0);  //try to read 0 index
	i2c_write(0); 

	i2c_restart(); 
	
	i2c_write(I2C_EEPROM_ADDRESS + 1); 
	i2c_read(0); 
	
	i2c_stop();
	
	return reponse;
}

unsigned char EE_byte_read(unsigned short word)
{
	unsigned char data;
 
    if(EEPROM == 0)
        return 0;
    
	i2c_start();
	i2c_write(I2C_EEPROM_ADDRESS); 

	i2c_write(word>>8); 
	i2c_write(word); 

	i2c_restart(); 
	
	i2c_write(I2C_EEPROM_ADDRESS + 1); 
	data = i2c_read(0); 
	
	i2c_stop(); 
	return data;
}


void EE_byte_write(unsigned short word,unsigned char data )
{
    if(EEPROM == 0)
        return;
	i2c_start(); 
	i2c_write(I2C_EEPROM_ADDRESS); 

	i2c_write(word>>8); 
	i2c_write(word); 

	i2c_write(data); 
	
	i2c_stop(); 
	__delay_ms(100);  //EEPROM WRITE DELAY
}
