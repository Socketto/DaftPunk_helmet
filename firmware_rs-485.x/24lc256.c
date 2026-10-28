#include "I2C.h"
#include "common.h"
#include <xc.h>
#include <pic18f2550.h>

#define I2C_EEPROM_WRITE_ADDRESS 0xA0
#define EEPROM_CAPACITY_BYTES 32768u
#define EEPROM_WRITE_TIMEOUT_MS 10u

unsigned char EE_Init()
{	
    unsigned char response;
    
	i2c_start();
    response = i2c_write(I2C_EEPROM_WRITE_ADDRESS);
    if(response == 0)
    {
        i2c_stop();
        return 0;
    }

    if(i2c_write(0) == 0 || i2c_write(0) == 0)
    {
        i2c_stop();
        return 0;
    }

	i2c_restart(); 
    if(i2c_write(I2C_EEPROM_WRITE_ADDRESS + 1) == 0)
    {
        i2c_stop();
        return 0;
    }

    i2c_read(0);
	i2c_stop();
	return 1;
}

unsigned char EE_byte_read(unsigned short word, unsigned char *data)
{
    if(EEPROM == 0 || word >= EEPROM_CAPACITY_BYTES)
    {
        return 0;
    }
    
	i2c_start();
    if(i2c_write(I2C_EEPROM_WRITE_ADDRESS) == 0 ||
       i2c_write((unsigned char)(word >> 8)) == 0 ||
       i2c_write((unsigned char)word) == 0)
    {
        i2c_stop();
        return 0;
    }
	i2c_restart(); 
    if(i2c_write(I2C_EEPROM_WRITE_ADDRESS + 1) == 0)
    {
        i2c_stop();
        return 0;
    }

    *data = i2c_read(0);
	i2c_stop(); 
	return 1;
}


unsigned char EE_byte_write(unsigned short word, unsigned char data)
{
    unsigned char retry;
    unsigned char acknowledged;

    if(EEPROM == 0 || word >= EEPROM_CAPACITY_BYTES)
    {
        return 0;
    }

	i2c_start(); 
    if(i2c_write(I2C_EEPROM_WRITE_ADDRESS) == 0 ||
       i2c_write((unsigned char)(word >> 8)) == 0 ||
       i2c_write((unsigned char)word) == 0 ||
       i2c_write(data) == 0)
    {
        i2c_stop();
        return 0;
    }
	
	i2c_stop(); 

    for(retry = 0; retry < EEPROM_WRITE_TIMEOUT_MS; retry++)
    {
        i2c_start();
        acknowledged = i2c_write(I2C_EEPROM_WRITE_ADDRESS);
        i2c_stop();
        if(acknowledged != 0)
        {
            return 1;
        }
        __delay_ms(1);
    }

    return 0;
}
