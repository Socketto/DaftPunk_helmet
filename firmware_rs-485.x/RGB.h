
#define BLACK   0x00
#define GREEN   0x01
#define RED     0x02
#define BLUE    0x04
#define VIOLET  BLUE|RED
#define YELLOW  RED|GREEN
#define CIAN    BLUE|GREEN
#define WHITE   BLUE|RED|GREEN



extern unsigned long counter;
extern unsigned long limit;
extern unsigned char color;

extern void SetRGB(unsigned char Color);
extern void SetRGB_Blink(unsigned char value);
extern void SetLed(char value);
extern void UpdateStatusLed();
extern void SetRGB_Blink(unsigned char value);
extern void SetEarRandom();