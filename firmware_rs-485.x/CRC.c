#define CRC16 0x8005

/*
unsigned short gen_crc16(const char *data, short size)
{  
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < size; pos++)
  {
  crc ^= (unsigned int)data[pos];    // XOR byte into least sig. byte of crc

  for (int i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }

  return crc;
}


unsigned char append_crc16(char *datat,unsigned char sizet)
{
    unsigned short tmpCRC = gen_crc16(datat, sizet);
    datat[sizet] = tmpCRC;
    datat[sizet+1] = tmpCRC>>8; 
    return (unsigned char)(sizet+2);
}
*/