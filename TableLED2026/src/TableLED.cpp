
#include "TableLED.h"
#include "Arduino.h"

CRGB leds[NUM_LEDS];
LiquidCrystal_I2C lcd(0x27, 20, 4);

// void InitWifiLib();

///////////////////////////////////////////////////////
//////////////////  Main functions  ///////////////////
///////////////////////////////////////////////////////

// Clears the led buffer
void TableLED::clear()
{
    memset(leds, 0, sizeof(leds));
}
// copies led buffer to desk
void TableLED::show()
{
    // neopixels->clear();
    for (int i = 0; i < NUM_LEDS; i++)
    {
        if (Processor != NULL)
        {
            if (!Processor(i))
            {
                neopixels->setPixelColor(i, neopixels->Color(0, 0, 0));
                continue;
            }
        }
        neopixels->setPixelColor(i, neopixels->Color(leds[i].r, leds[i].g, leds[i].b));
    }
    /*#if WIFI_EN
        WCpyLEDBuffer();
    #endif*/
    neopixels->show();
}

// Initiates the library
void TableLED::begin()
{
    neopixels = new Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
    neopixels->begin();

    clear();

    neopixels->clear();
    neopixels->show();

    lcd.init();
    lcd.clear();
    lcd.backlight();

    // StartSD();
    Joystick::Begin();

    MCP::Start();
}

///////////////////////////////////////////////////////
///////////////  RGB based functions  /////////////////
///////////////////////////////////////////////////////

void TableLED::ShowCol(byte col, CRGB color)
{
    for (byte i = 0; i < ROW_COUNT; i++)
    {
        if (ROW_COL_TABLE[i][col] == 255)
            continue;
        leds[ROW_COL_TABLE[i][col]] = color;
    }
}
void TableLED::ShowRow(byte row, CRGB color)
{
    for (byte i = 0; i < COLUMN_COUNT; i++)
    {
        if (ROW_COL_TABLE[row][i] == 255)
            continue;
        leds[ROW_COL_TABLE[row][i]] = color;
    }
}
void TableLED::ShowHexagon(byte hexagon, CRGB color)
{
    for (byte i = 0; i < LEDS_PER_HEXAGON; i++)
    {
        leds[HEXAGON_TABLE[hexagon][i]] = color;
    }
}
void TableLED::Fill(CRGB color)
{
    for (byte i = 0; i < 245; i++)
    {
        leds[i] = color;
    }
}

///////////////////////////////////////////////////////
///////////////  HSV based functions  /////////////////
///////////////////////////////////////////////////////

CRGB TableLED::ToRGB(uint32_t color)
{
    CRGB output;
    output.r = (uint8_t)((color >> 16) & 0xFF);
    output.g = (uint8_t)((color >> 8) & 0xFF);
    output.b = (uint8_t)(color & 0xFF);
    return output;
}
CRGB TableLED::ToRGB(CHSV color)
{
    uint32_t packedColor = (neopixels->ColorHSV(color.h * 257, color.s, color.v));
    CRGB output;
    output.r = (uint8_t)((packedColor >> 16) & 0xFF);
    output.g = (uint8_t)((packedColor >> 8) & 0xFF);
    output.b = (uint8_t)(packedColor & 0xFF);
    return output;
}
void TableLED::ShowCol(byte col, CHSV color)
{
    ShowCol(col, ToRGB(color));
}
void TableLED::ShowRow(byte row, CHSV color)
{
    ShowRow(row, ToRGB(color));
}
void TableLED::ShowHexagon(byte hex, CHSV color)
{
    ShowHexagon(hex, ToRGB(color));
}
void TableLED::Fill(CHSV color)
{
    Fill(ToRGB(color));
}

///////////////////////////////////////////////////////
/////////////////  Other functions  ///////////////////
///////////////////////////////////////////////////////

void TableLED::Fade(float multiplier)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = ToRGB(neopixels->getPixelColor(i));
        leds[i].r *= multiplier;
        leds[i].g *= multiplier;
        leds[i].b *= multiplier;
    }
}

bool TableLED::CompareColor(CRGB c1, CRGB c2)
{
    if (c1.r != c2.r)
        return false;
    if (c1.g != c2.g)
        return false;
    if (c1.b != c2.b)
        return false;
    return true;
}

CRGB TableLED::GetLED(byte index)
{
    return ToRGB(neopixels->getPixelColor(index));
}

float TableLED::LedDistance(byte a, byte b)
{
    return sqrt(pow(ledData[a].row - ledData[b].row, 2) + pow(ledData[a].col - ledData[b].col, 2));
}
float TableLED::LedDistance(byte a, byte b, byte multiplier)
{
    return sqrt(pow((float)ledData[b].col - (float)ledData[a].col, 2) + pow((float)ledData[b].row - (float)ledData[a].row, 2)) * multiplier;
}

CHSV TableLED::AverageColors(CHSV a, CHSV b)
{
    CHSV out;
    if (abs(a.h - b.h) > 128)
    {
        out.h = ((int)((a.h + b.h + 255) / 2)) % 255;
    }
    else
    {
        out.h = (a.h + b.h) / 2;
    }
    out.s = (a.s + b.s) / 2;
    out.v = (a.v + b.v) / 2;
    return out;
}
CHSV TableLED::LerpColors(CHSV a, CHSV b, float t, bool wrapAround)
{
    CHSV out;
    if (wrapAround)
    {
        if (abs(a.h - b.h) > 128)
        {
            if (b.h < a.h)
            {
                out.h = a.h + (b.h + 255 - a.h) * t;
            }
            else
            {
                out.h = a.h + (b.h - 255 - a.h) * t;
            }
        }
        else
        {
            out.h = a.h + (b.h - a.h) * t;
        }
        out.s = a.s * (1 - t) + b.s * t;
        out.v = a.v * (1 - t) + b.v * t;
    }
    else
    {
        out.h = a.h + ((float)(b.h - a.h)) * t;
        out.s = a.s + ((float)(b.s - a.s)) * t;
        out.v = a.v + ((float)(b.v - a.v)) * t;
    }
    return out;
}
CRGB TableLED::LerpColors(CRGB a, CRGB b, float t)
{
    CRGB output;
    output.r = a.r + ((float)(b.r - a.r)) * t;
    output.g = a.g + ((float)(b.g - a.g)) * t;
    output.b = a.b + ((float)(b.b - a.b)) * t;
    return output;
}

bool TableLED::PrintChar(char character, byte pos, CRGB color)
{
    for (int i = 0; i < sizeof(DeskFont::font) / sizeof(DeskFont::Character); i++)
    {
        if (DeskFont::font[i].asciiNumber == character)
        {
            for (uint x = 0; x < 21; x++)
            {
                if ((DeskFont::font[i].data & (1 << (31 - x))) != 0)
                {
                    leds[ROW_COL_TABLE
                             [ledData[pos].row - DeskFont::charToY[x]]
                             [ledData[pos].col + DeskFont::charToX[x]]] = color;
                }
            }
            return true;
        }
    }
    return false;
}