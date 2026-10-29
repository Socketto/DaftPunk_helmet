#include "I2C.h"
#include "common.h"
#include <xc.h>
#include <pic18f2550.h>

#define I2C_SDA_TRIS TRISBbits.TRISB0
#define I2C_SCL_TRIS TRISBbits.TRISB1
#define I2C_SDA_READ PORTBbits.RB0
#define I2C_SCL_READ PORTBbits.RB1
#define I2C_CLOCK_TIMEOUT_US 100
#define I2C_HALF_PERIOD_US 5

static void i2c_sda_low(void)
{
    I2C_SDA_TRIS = 0;
}

static void i2c_sda_release(void)
{
    I2C_SDA_TRIS = 1;
}

static void i2c_scl_low(void)
{
    I2C_SCL_TRIS = 0;
}

static void i2c_scl_release(void)
{
    I2C_SCL_TRIS = 1;
}

static unsigned char i2c_wait_scl_high(void)
{
    unsigned char timeout = I2C_CLOCK_TIMEOUT_US;

    while(!I2C_SCL_READ && timeout > 0)
    {
        __delay_us(1);
        timeout--;
    }

    return (unsigned char)(I2C_SCL_READ != 0);
}

void i2c_init(void)
{
    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;
    i2c_sda_release();
    i2c_scl_release();
}

unsigned char i2c_idle(void)
{
    i2c_sda_release();
    i2c_scl_release();

    if(!i2c_wait_scl_high())
    {
        return 0;
    }

    __delay_us(I2C_HALF_PERIOD_US);
    return (unsigned char)(I2C_SDA_READ != 0);
}

unsigned char i2c_start(void)
{
    if(!i2c_idle())
    {
        return 0;
    }

    i2c_sda_low();
    __delay_us(I2C_HALF_PERIOD_US);
    i2c_scl_low();
    __delay_us(I2C_HALF_PERIOD_US);
    return 1;
}

unsigned char i2c_restart(void)
{
    i2c_sda_release();
    __delay_us(I2C_HALF_PERIOD_US);
    i2c_scl_release();

    if(!i2c_wait_scl_high())
    {
        i2c_scl_low();
        return 0;
    }

    __delay_us(I2C_HALF_PERIOD_US);
    i2c_sda_low();
    __delay_us(I2C_HALF_PERIOD_US);
    i2c_scl_low();
    __delay_us(I2C_HALF_PERIOD_US);
    return 1;
}

unsigned char i2c_write(unsigned char data)
{
    unsigned char bit;

    for(bit = 0; bit < 8; bit++)
    {
        if(data & 0x80)
        {
            i2c_sda_release();
        }
        else
        {
            i2c_sda_low();
        }

        __delay_us(I2C_HALF_PERIOD_US);
        i2c_scl_release();
        if(!i2c_wait_scl_high())
        {
            i2c_scl_low();
            i2c_sda_release();
            return 0;
        }

        __delay_us(I2C_HALF_PERIOD_US);
        i2c_scl_low();
        data <<= 1;
    }

    i2c_sda_release();
    __delay_us(I2C_HALF_PERIOD_US);
    i2c_scl_release();
    if(!i2c_wait_scl_high())
    {
        i2c_scl_low();
        return 0;
    }

    __delay_us(I2C_HALF_PERIOD_US);
    bit = (unsigned char)(I2C_SDA_READ == 0);
    i2c_scl_low();
    return bit;
}

unsigned char i2c_read(unsigned char ack, unsigned char *data)
{
    unsigned char bit;
    unsigned char receivedData = 0;

    if(data == 0)
    {
        return 0;
    }

    i2c_sda_release();
    for(bit = 0; bit < 8; bit++)
    {
        receivedData <<= 1;
        i2c_scl_release();
        if(!i2c_wait_scl_high())
        {
            i2c_scl_low();
            return 0;
        }

        __delay_us(I2C_HALF_PERIOD_US);
        if(I2C_SDA_READ)
        {
            receivedData |= 1;
        }
        i2c_scl_low();
        __delay_us(I2C_HALF_PERIOD_US);
    }

    if(ack)
    {
        i2c_sda_low();
    }
    else
    {
        i2c_sda_release();
    }

    __delay_us(I2C_HALF_PERIOD_US);
    i2c_scl_release();
    if(!i2c_wait_scl_high())
    {
        i2c_scl_low();
        i2c_sda_release();
        return 0;
    }

    __delay_us(I2C_HALF_PERIOD_US);
    i2c_scl_low();
    i2c_sda_release();
    *data = receivedData;
    return 1;
}

unsigned char i2c_stop(void)
{
    unsigned char clockReleased;

    i2c_sda_low();
    __delay_us(I2C_HALF_PERIOD_US);
    i2c_scl_release();
    clockReleased = i2c_wait_scl_high();
    if(clockReleased)
    {
        __delay_us(I2C_HALF_PERIOD_US);
    }
    i2c_sda_release();
    __delay_us(I2C_HALF_PERIOD_US);
    return clockReleased;
}
