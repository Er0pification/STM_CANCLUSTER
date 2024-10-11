/*********************************************************************
This is an example for our Monochrome OLEDs based on SH1106 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using SPI to communicate
4 or 5 pins are required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

// If using software SPI (the default case):
#define OLED_MOSI   PB5
#define OLED_CLK   PB3
#define OLED_DC    PB6
#define OLED_CS    PB7
#define OLED_RESET PB4
Adafruit_SH1106 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Uncomment this block to use hardware SPI
/*#define OLED_DC     PB6
#define OLED_CS     PB7
#define OLED_RESET  PB4
Adafruit_SH1106 display(OLED_DC, OLED_RESET, OLED_CS);
*/

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

static const unsigned char PROGMEM RUSEFI [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFE, 0x0F, 0x80,
0x00, 0x7C, 0x00, 0xFF, 0xFF, 0xC0, 0x07, 0xFF, 0xFE, 0x0F, 0x80, 0x00, 0x7C, 0x03, 0xFF, 0xFF,
0xC0, 0x0F, 0xFF, 0xFE, 0x0F, 0x80, 0x00, 0x7C, 0x07, 0xFF, 0xFF, 0xC0, 0x1F, 0xFF, 0xFE, 0x0F,
0x80, 0x00, 0x7C, 0x0F, 0xFF, 0xFF, 0xC0, 0x1F, 0xFF, 0xFE, 0x0F, 0x80, 0x00, 0x7C, 0x0F, 0xFF,
0xFF, 0xC0, 0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C, 0x1F, 0x80, 0x00, 0x00, 0x3E, 0x00, 0x00,
0x0F, 0x80, 0x00, 0x7C, 0x1F, 0x80, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C, 0x1F,
0x80, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C, 0x1F, 0xFF, 0xFE, 0x00, 0x3E, 0x00,
0x00, 0x0F, 0x80, 0x00, 0x7C, 0x0F, 0xFF, 0xFF, 0x80, 0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C,
0x0F, 0xFF, 0xFF, 0xC0, 0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C, 0x07, 0xFF, 0xFF, 0xE0, 0x3E,
0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C, 0x03, 0xFF, 0xFF, 0xF0, 0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00,
0x7C, 0x00, 0xFF, 0xFF, 0xF0, 0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C, 0x00, 0x00, 0x03, 0xF8,
0x3E, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x7C, 0x00, 0x00, 0x01, 0xF8, 0x3E, 0x00, 0x00, 0x0F, 0x80,
0x00, 0x7C, 0x00, 0x00, 0x01, 0xF8, 0x3E, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x7C, 0x00, 0x00, 0x01,
0xF8, 0x3E, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x7C, 0x00, 0x00, 0x03, 0xF8, 0x3E, 0x00, 0x00, 0x07,
0xFF, 0xFF, 0xFC, 0x0F, 0xFF, 0xFF, 0xF0, 0x3E, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFC, 0x0F, 0xFF,
0xFF, 0xF0, 0x3E, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFC, 0x0F, 0xFF, 0xFF, 0xE0, 0x3E, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0xFC, 0x0F, 0xFF, 0xFF, 0xC0, 0x3E, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFC, 0x0F,
0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xE0, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xC0, 0x0F,
0xFF, 0xFF, 0xFF, 0xE0, 0xFF, 0xFF, 0xFF, 0xFE, 0x7F, 0xC0, 0x1C, 0x00, 0x00, 0x00, 0x61, 0xC0,
0x00, 0x00, 0x06, 0x60, 0xC0, 0x38, 0x00, 0x00, 0x00, 0x63, 0x80, 0x00, 0x00, 0x06, 0x60, 0xC0,
0x30, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00, 0x06, 0x60, 0xC0, 0x30, 0x00, 0x00, 0x00, 0x63,
0x00, 0x00, 0x00, 0x06, 0x60, 0xC0, 0x30, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00, 0x06, 0x60,
0xC0, 0x30, 0x7F, 0xFF, 0xFF, 0xE3, 0x07, 0xFF, 0xFF, 0xFE, 0x60, 0xC0, 0x30, 0x7F, 0xFF, 0xFF,
0xE3, 0x07, 0xFF, 0xFF, 0xFE, 0x60, 0xC0, 0x30, 0x60, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00,
0x60, 0xC0, 0x30, 0x60, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x30, 0x60, 0x00,
0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x30, 0x60, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00,
0x00, 0x60, 0xC0, 0x30, 0x60, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x30, 0x7F,
0xFF, 0xFE, 0x03, 0x07, 0xFF, 0xFF, 0xE0, 0x60, 0xC0, 0x30, 0x7F, 0xFF, 0xFE, 0x03, 0x07, 0xFF,
0xFF, 0xE0, 0x60, 0xC0, 0x30, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x60, 0x60, 0xC0, 0x30,
0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x60, 0x60, 0xC0, 0x30, 0x00, 0x00, 0x06, 0x03, 0x00,
0x00, 0x00, 0x60, 0x60, 0xC0, 0x30, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x60, 0x60, 0xC0,
0x30, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x60, 0x60, 0xC0, 0x30, 0x7F, 0xFF, 0xFE, 0x03,
0x07, 0xFF, 0xFF, 0xE0, 0x60, 0xC0, 0x30, 0x7F, 0xFF, 0xFE, 0x03, 0x07, 0xFF, 0xFF, 0xE0, 0x60,
0xC0, 0x30, 0x60, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x30, 0x60, 0x00, 0x00,
0x03, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x30, 0x60, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00,
0x60, 0xC0, 0x30, 0x60, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x30, 0x7F, 0xFF,
0xFF, 0xE3, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x30, 0x7F, 0xFF, 0xFF, 0xE3, 0x06, 0x00, 0x00,
0x00, 0x60, 0xC0, 0x30, 0x00, 0x00, 0x00, 0x63, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x18, 0x00,
0x00, 0x00, 0x63, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x1C, 0x00, 0x00, 0x00, 0x63, 0x06, 0x00,
0x00, 0x00, 0x60, 0xC0, 0x0E, 0x00, 0x00, 0x00, 0x63, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x07,
0x00, 0x00, 0x00, 0x63, 0x06, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x03, 0xFF, 0xFF, 0xFF, 0xE3, 0xFE,
0x00, 0x00, 0x00, 0x7F, 0xC0, 0x01, 0xFF, 0xFF, 0xFF, 0xE3, 0xFE, 0x00, 0x00, 0x00, 0x7F, 0xC0
};

void lcd_terminal (char * string, uint8_t isinverted)
{
  static char string3[25];
  static char string2[25];
  static char string1[25];
  strcpy (string3,string2);
  strcpy (string2,string1);
  strcpy (string1,string);
  display.fillRect(0,40,128,24,0);
  display.setCursor(0,40);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(string3);
  display.println(string2);
  display.println(string1);
  display.display();
}


void lcd_setup()   {        
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SH1106_SWITCHCAPVCC);
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.c
  display.setRotation(2);
  display.clearDisplay();
  display.drawBitmap(20,0, RUSEFI,88,64,1);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();

}


