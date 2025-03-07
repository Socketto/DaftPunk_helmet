
extern void i2c_init(void);
extern void mssp_wait(void);
extern void i2c_restart(void);
extern void i2c_idle(void);
extern void i2c_start(void);
extern unsigned char i2c_write(unsigned char data);
extern unsigned char i2c_read( unsigned char ack );
extern void i2c_ack(void);
extern void i2c_nack(void);
extern void i2c_stop(void);