#include "common.h"

extern unsigned char EE_Init();
extern unsigned char EE_byte_read(unsigned short word, unsigned char *data);
extern unsigned char EE_byte_write(unsigned short word, unsigned char data);