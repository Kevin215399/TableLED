
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#if WIFI_EN
#include <TableWiFi.h>
#endif

// #include <TableLED_SD.h>
#include "TableSD.h"
#include "Font.h"

#define LED_PIN 32
#define NUM_LEDS 245

#define COLUMN_COUNT 23
#define ROW_COUNT 17

#define LEDS_PER_HEXAGON 12
#define HEXAGON_COUNT 38

#include "Joystick.h"

struct CRGB
{
  byte r;
  byte g;
  byte b;

  CRGB(byte _r, byte _g, byte _b)
  {
    r = _r;
    g = _g;
    b = _b;
  }
  CRGB()
  {
    r = 0;
    g = 0;
    b = 0;
  }

  inline bool operator==(const CRGB &c2)
  {
    if (r != c2.r)
      return false;
    if (g != c2.g)
      return false;
    if (b != c2.b)
      return false;
    return true;
  }

  inline bool operator!=(const CRGB &c2)
  {
    if (r != c2.r)
      return true;
    if (g != c2.g)
      return true;
    if (b != c2.b)
      return true;
    return false;
  }
};

struct CHSV
{
  byte h;
  byte s;
  byte v;

  CHSV(byte _h, byte _s, byte _v)
  {
    h = _h;
    s = _s;
    v = _v;
  }
  CHSV()
  {
    h = 0;
    s = 0;
    v = 0;
  }
  inline bool operator==(const CHSV &c2)
  {
    if (h != c2.h)
      return false;
    if (s != c2.s)
      return false;
    if (v != c2.v)
      return false;
    return true;
  }

  inline bool operator!=(const CHSV &c2)
  {
    if (h != c2.h)
      return true;
    if (s != c2.s)
      return true;
    if (v != c2.v)
      return true;
    return false;
  }
};

struct LedData
{
  byte col;
  byte row;
  int16_t up;
  int16_t down;
  int16_t left;
  int16_t right;
  int16_t hexaA;
  int16_t hexaB;
  int16_t hexaC;

  LedData(
      byte _col,
      byte _row,
      int16_t _up,
      int16_t _down,
      int16_t _left,
      int16_t _right,
      int16_t _hexaA,
      int16_t _hexaB,
      int16_t _hexaC)
  {
    col = _col;
    row = _row;
    up = _up;
    down = _down;
    left = _left;
    right = _right;
    hexaA = _hexaA;
    hexaB = _hexaB;
    hexaC = _hexaC;
  }

  LedData()
  {
    col = 0;
    row = 0;
    up = 0;
    down = 0;
    left = 0;
    right = 0;
    hexaA = 0;
    hexaB = 0;
    hexaC = 0;
  }
};

class TableLED
{
public:
  bool (*Processor)(byte);

  void begin();
  void show();
  void clear();

  void ShowCol(byte col, CRGB color);
  void ShowRow(byte row, CRGB color);
  void ShowHexagon(byte hex, CRGB color);

  void ShowCol(byte col, CHSV color);
  void ShowRow(byte row, CHSV color);
  void ShowHexagon(byte hex, CHSV color);

  // Completely fill desk with color
  void Fill(CRGB color);
  // Completely fill desk with color
  void Fill(CHSV color);

  // Multiply the RGB value of each led by a number
  void Fade(float multiplier);

  // Convert HSV to RGB
  CRGB ToRGB(CHSV color);
  // Convert a uint32_t to a RGB value
  CRGB ToRGB(uint32_t color);
  // Get the color of an LED
  CRGB GetLED(byte index);
  // Find the distance between 2 leds using distance formula
  float LedDistance(byte a, byte b);
  // Find the distance between 2 leds using distance formula
  float LedDistance(byte a, byte b, byte multiplier);
  // Check if two colors are equal
  bool CompareColor(CRGB c1, CRGB c2);
  // Average the colors
  CHSV AverageColors(CHSV a, CHSV b);
  // Lerp colors between 0->1, a->b
  CHSV LerpColors(CHSV a, CHSV b, float t, bool wrapAround);
  // lerp RGB colors
  CRGB LerpColors(CRGB a, CRGB b, float t);

  /*
  * @brief print a character
  * @param character The character to print
  * @param pos The bottom left point of the character
  * @return True if the print succeeded
  */
  bool PrintChar(char character, byte pos, CRGB color);

  Adafruit_NeoPixel *neopixels = nullptr;
};

// Led declarations
extern CRGB leds[NUM_LEDS];
extern const byte ROW_COL_TABLE[17][23];
extern const byte HEXAGON_TABLE[38][12];
extern LedData ledData[245];
bool CompareColor(CRGB c1, CRGB c2);

// LCD declarations
extern LiquidCrystal_I2C lcd;

// Joystick Declarations
void InitJoystick();
int JoystickX();
int JoystickY();
bool JoystickDown();

// MCP declerations
#include "MCPExpander.h"
