/*
 An example showing 'ring' analogue meters on a 2.2" round TFT 128x128 ST7687S
 colour screen
 Based on https://www.instructables.com/id/Arduino-analogue-ring-meter-on-colour-TFT-display/
 and dfrobot ST7687s lobrary  https://github.com/DFRobot/DFRobot_ST7687S
 */


// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

#include "DFRobot_ST7687S_Latch.h"
#include "DFRobot_Display_Clock.h"
#include <SPI.h>

#define ILI9341_GREY 0x2104                                                     // Dark grey 16 bit colour

#ifdef __AVR__
uint8_t pin_cs = 10, pin_rs = 5, pin_wr = 6, pin_lck = 7;
#else
uint8_t pin_cs = 26, pin_rs = 16, pin_wr = 4, pin_lck = 13;
#endif

DFRobot_ST7687S_Latch tft(pin_cs, pin_rs, pin_wr, pin_lck);

uint32_t runTime = -99999;                                                      // time for next update

int reading = 0;                                                                // Value to be displayed
int d = 0;                                                                      // Variable used for the sinewave test waveform

void setup(void) {
  tft.begin();
//tft.setRotation(3);
  tft.fillScreen(DISPLAY_BLACK);
}

void loop() {
  if (millis() - runTime >= 1000L) {                                            // Execute every 2s
    runTime = millis();

    // Test with a slowly changing value from a Sine function
    d += 5; if (d >= 360) d = 0;
    
    // Test with Sine wave function, normally reading will be from a sensor
    reading = 100 + 100 * sineWave(d+0);
    ringMeter(reading,0,200,-60,-60,60," Watts",BLUE2RED);                      // Draw analogue meter

  }
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;                                                               // Calculate coords of centre of ring
  int w = r / 4;                                                                // Width of outer ring is 1/4 of radius
  int angle = 150;                                                              // Half the sweep angle of meter (300 degrees)
  int text_colour = 0;                                                          // To hold the text colour
  int v = map(value, vmin, vmax, -angle, angle);                                // Map the value to an angle v

  byte seg = 5;                                                                 // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 10;                                                                // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = DISPLAY_RED; break; // Fixed colour
      case 1: colour = DISPLAY_GREEN; break; // Fixed colour
      case 2: colour = DISPLAY_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break;           // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break;          // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break;          // Red to green (low battery etc)
      default: colour = DISPLAY_BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_GREY);
    }
  }

  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);

  // Set the text colour to default
  tft.setTextColor(DISPLAY_WHITE);
  tft.setTextBackground(DISPLAY_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  // tft.setTextColor(text_colour, ILI9341_BLACK);
  
  // Print value, if the meter is large then use big font 6, othewise use 4
  if (r > 84){ 
  tft.setTextSize(2); //2 * text size, default text size: 6 * 8
  tft.setCursor(x-5, y-20);  //set text position to center
  tft.print(buf);
  //tft.drawCentreString(buf, x - 5, y - 20, 6); // Value in middle
  }
  else {
  tft.setTextSize(2); //2 * text size, default text size: 6 * 8
  tft.setCursor(x+35, y+70);  //set text position to center
  tft.print(buf);
  //tft.drawCentreString(buf, x - 5, y - 20, 4); // Value in middle
  }

  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(DISPLAY_WHITE);
  tft.setTextBackground(DISPLAY_BLACK);
  if (r > 84) {
  tft.setTextSize(2); //2 * text size, default text size: 6 * 8
  tft.setCursor(x, y+30);  //set text position to center
  tft.print(units);
  //tft.drawCentreString(units, x, y + 30, 4); // Units display
  }
  else {
  tft.setTextSize(2); //2 * text size, default text size: 6 * 8
  tft.setCursor(x+25, y+50);  //set text position to center
  tft.print(units);
  //tft.drawCentreString(units, x, y + 5, 2); // Units display
  }

  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0;                                                                 // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;                                                               // Green is the middle 6 bits
  byte blue = 0;                                                                // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}
