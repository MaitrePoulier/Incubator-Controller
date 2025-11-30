#include <Arduino.h>
//#include "Alert.h" // Out of range alert icon
#include <TFT_eSPI.h> 

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5
#define TFT_GREY 0x2104 // Dark grey 16 bit colour

void tft_init(void);
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, const char *units, byte scheme);
//void drawAlert(int x, int y, int side, bool draw);
unsigned int rainbow(byte value);
float sineWave(int phase);
void drawIcon(const unsigned short *icon, int16_t x, int16_t y, int8_t width, int8_t height);
