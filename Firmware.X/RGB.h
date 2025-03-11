
#define BLACK   0x00
#define GREEN   0x01
#define BLUE    0x02
#define RED     0x04
#define VIOLET  BLUE|RED
#define YELLOW  RED|GREEN
#define CIAN    BLUE|GREEN
#define WHITE   BLUE|RED|GREEN

extern void SetRGB(unsigned Color);
extern void SetLed(char value);