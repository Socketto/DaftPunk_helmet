
extern void i2c_init(void);
extern unsigned char i2c_restart(void);
extern unsigned char i2c_idle(void);
extern unsigned char i2c_start(void);
extern unsigned char i2c_write(unsigned char data);
extern unsigned char i2c_read(unsigned char ack, unsigned char *data);
extern unsigned char i2c_stop(void);