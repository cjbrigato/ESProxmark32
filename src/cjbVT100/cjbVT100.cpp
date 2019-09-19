#include "common/common_firmware.h"
#include "font6x8.h"

// The scrolling area must be a integral multiple of TEXT_HEIGHT
#define TEXT_HEIGHT 9    // Height of text to be printed and scrolled
#define TOP_FIXED_AREA 9 // Number of lines in top fixed area (lines counted from top of screen)
#define BOT_FIXED_AREA 42 // Number of lines in bottom fixed area (lines counted from bottom of screen)
#define YMAX 240         // Bottom of screen area

uint16_t yStart = 0;
uint16_t yArea = YMAX - TOP_FIXED_AREA - BOT_FIXED_AREA;
uint16_t yDraw = YMAX - BOT_FIXED_AREA - TEXT_HEIGHT;
//uint16_t yDraw = 0;

uint16_t xPos = 0;


boolean change_colour = 1;
boolean selected = 1;

// We have to blank the top line each time the display is scrolled, but this takes up to 13 milliseconds
// for a full width line, meanwhile the serial buffer may be filling... and overflowing
// We can speed up scrolling of short text lines by just blanking the character we drew
int blank[19]; // We keep all the strings pixel lengths to optimise the speed of the top line blanking



int xp = 0;
int yp = 0;
uint16_t bg = FAKEBLACK;
uint16_t fg = ILI9341_WHITE;
int screenWd = 320;
//int screenHt = 240;
int screenHt = 240;
int wrap = 0;
int bold = 0;
int sx = 1;
int sy = 1;
int horizontal = -1;
int scrollMode = 1;

int get_scroll_mode()
{
  return scrollMode;
}

// ##############################################################################################
// Setup a portion of the screen for vertical scrolling
// ##############################################################################################
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void setupScrollArea(uint16_t tfa, uint16_t bfa)
{
  M5.Lcd.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
  M5.Lcd.writedata(tfa >> 8);           // Top Fixed Area line count
  M5.Lcd.writedata(tfa);
  M5.Lcd.writedata((YMAX - tfa - bfa) >> 8); // Vertical Scrolling Area line count
  M5.Lcd.writedata(YMAX - tfa - bfa);
  M5.Lcd.writedata(bfa >> 8); // Bottom Fixed Area line count
  M5.Lcd.writedata(bfa);

  /*     M5.Lcd.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
 M5.Lcd.writedata(tfa >> 8);
  M5.Lcd.writedata(tfa);
 M5.Lcd.writedata((320 - tfa - bfa) >> 8);
M5.Lcd.writedata(320 - tfa - bfa);
M5.Lcd.writedata(bfa >> 8);
 M5.Lcd.writedata(bfa);*/
}

// ##############################################################################################
// Setup the vertical scrolling start address pointer
// ##############################################################################################
void scrollAddress(uint16_t vsp)
{
  M5.Lcd.writecommand(ILI9341_VSCRSADD); // Vertical scrolling pointer
  M5.Lcd.writedata(vsp >> 8);
  M5.Lcd.writedata(vsp);
}

/*
CONNECTIONS:
------------
STM32:
For programming via serial:
Tools/Board set to Generic STM32F103C
Tools/Upload set to Serial
Top jumper set to 1, press the button before uploading
  PA9 /TX to PC RX (VIOLET)
  PA10/RX to PC TX (GREY)
  3V3              (RED)
  GND              (BLUE)
 STM32 SPI1 pins:
  PA4 CS1
  PA5 SCK1
  PA6 MISO1
  PA7 MOSI1
  PA11 RST
  PA12 DC
TFT2.2 ILI9341 from top left:
  MISO  PA6
  LED   +3.3V
  SCK   PA5
  MOSI  PA7
  DC    PA12
  RST   PA11 or +3V3
  CS    PA4
  GND   GND
  VCC   +3.3V
*/

// Uncomment below the font you find the most readable for you
// 7x8 bold - perfect for small term font
//#include "font_b7x8.h"
//const uint16_t *fontRects = font_b7x8_Rects;
//const uint16_t *fontOffs = font_b7x8_CharOffs;
//int charWd = 7;
//int charHt = 10; // real 8
//int charYoffs = 1;

// 7x8 - perfect for small terminal font
//#include "font_7x8.h"
//const uint16_t *fontRects = font_7x8_Rects;
//const uint16_t *fontOffs = font_7x8_CharOffs;
//int charWd = 7;
//int charHt = 10; // real 8
//int charYoffs = 1;

// 6x8
//#include "font_6x8.h"
//const uint16_t *fontRects = font_6x8_Rects;
//const uint16_t *fontOffs = font_6x8_CharOffs;
//int charWd = 6;
//int charHt = 9; // real 8
//int charYoffs = 1;

// nice 8x16 vga terminal font
//#include "font_term_8x16.h"
//const uint16_t *fontRects = wlcd_font_term_8x16_0_127_Rects;
//const uint16_t *fontOffs = wlcd_font_term_8x16_0_127_CharOffs;
//int charWd = 8;
//int charHt = 16;
//int charYoffs = 0;

// nice big for terminal
//#include "font_fxs_8x15.h"
//const uint16_t *fontRects = wlcd_font_fxs_8x15_16_127_Rects;
//const uint16_t *fontOffs = wlcd_font_fxs_8x15_16_127_CharOffs;
//int charWd = 8;
//int charHt = 15; // real 15
//int charYoffs = 0;

// my nice 10x16 term
//#include "font_term_10x16.h"
//const uint16_t *fontRects = font_term_10x16_Rects;
//const uint16_t *fontOffs = font_term_10x16_CharOffs;
//int charWd = 10;
//int charHt = 16;
//int charYoffs = 0;

/*  M5.Lcd.setTextColor(TFT_WHITE, TFT_RED);
  M5.Lcd.fillRect(0, 0, 320, TEXT_HEIGHT, TFT_RED);
  M5.Lcd.drawCentreString(" PM3RDV40 ESP32 READY", 320 / 2, 0, 2);*/

void drawChar(int16_t x, int16_t y, unsigned char c,
              uint16_t color, uint16_t bg, uint8_t sx, uint8_t sy)
{
  if ((x >= screenWd) ||             // Clip right
      (y >= screenHt) ||             // Clip bottom
      ((x + charWd * sx - 1) < 0) || // Clip left
      ((y + charHt * sy - 1) < 0))   // Clip top
    return;
  if (c > 127)
    return;
  uint16_t recIdx = fontOffs[c];
  uint16_t recNum = fontOffs[c + 1] - recIdx;
  if (bg && bg != color)
    M5.Lcd.fillRect(x, y, charWd * sx, charHt * sy, bg);
  if (charWd <= 16 && charHt <= 16)
    for (int i = 0; i < recNum; i++)
    {
      int v = fontRects[i + recIdx];
      int xf = v & 0xf;
      int yf = charYoffs + ((v & 0xf0) >> 4);
      int wf = 1 + ((v & 0xf00) >> 8);
      int hf = 1 + ((v & 0xf000) >> 12);
      M5.Lcd.fillRect(x + xf * sx, y + yf * sy, bold + wf * sx, hf * sy, color);
    }
  else
    for (int i = 0; i < recNum; i++)
    {
      uint8_t *rects = (uint8_t *)fontRects;
      int idx = (i + recIdx) * 3;
      int xf = rects[idx + 0] & 0x3f;
      int yf = rects[idx + 1] & 0x3f;
      int wf = (1 + rects[idx + 2]) & 0x3f;
      //int wf = 1 + rects[idx + 2] & 0x3f;
      int hf = 1 + (((rects[idx + 0] & 0xc0) >> 6) | ((rects[idx + 1] & 0xc0) >> 4) | ((rects[idx + 2] & 0xc0) >> 2));
      M5.Lcd.fillRect(x + xf * sx, y + yf * sy, bold + wf * sx, hf * sy, color);
    }
}

void scroll()
{
  xp = 0;
  yp += charHt * sy;
  //if(yp+charHt>screenHt) yp=0;

  if (yp >= screenHt-BOT_FIXED_AREA-(charHt*sy))
  {
    scrollMode = 1;
  }

  if (yp > screenHt-BOT_FIXED_AREA-(charHt*sy))
    yp =0+TOP_FIXED_AREA;
  M5.Lcd.fillRect(0, yp, screenWd, charHt * sy, FAKEBLACK);


  if (scrollMode)
  {
    scrollFrame(yp + charHt*sy);
  }
}

int escMode = 0;
int nVals = 0;
int vals[10] = {0};

void printChar(char c)
{
  if (c == 0x1b)
  {
    escMode = 1;
    return;
  }
  if (escMode == 1)
  {
    if (c == '[')
    {
      escMode = 2;
      nVals = 0;
    }
    else
      escMode = 0;
    return;
  }
  if (escMode >= 3)
  {
    if (c == 0x1b)
    {
      escMode++;
      return;
    }
    if (escMode >= 4)
    {
      if (c == '[')
      {
        escMode++;
        return;
      }
      if (escMode >= 5)
      {
        if (c == 0x32)
        {
          escMode++;
          M5.Lcd.clear(FAKEBLACK);
          M5.Lcd.setCursor(0, 0);
          xp = 0;
          yp = 0;
          scrollFrame(0);
          escMode = 0;
          return;
        }
        if (escMode >= 6)
        {
          if (c == 0x4a)
          {
            M5.Lcd.clear(FAKEBLACK);
            M5.Lcd.setCursor(0, 0);
            xp = 0;
            yp = 0;
            scrollFrame(0);
            escMode = 0;
          }
        }
      }
    }
  }
  if (escMode == 2)
  {
    if (isdigit(c))
      vals[nVals] = vals[nVals] * 10 + (c - '0');
    else if (c == ';')
      nVals++;
    else if (c == 'H')
    {
      escMode = 3;
      return;
    }
    else if (c == 'm')
    {
      escMode = 0;
      nVals++;
      for (int i = 0; i < nVals; i++)
      {
        int v = vals[i];
        static const uint16_t colors[] = {
            0x0000, // 0-black
            0xf800, // 1-red
            0x0780, // 2-green
            0xfe00, // 3-yellow
            0x001f, // 4-blue
            0xf81f, // 5-magenta
            0x07ff, // 6-cyan
            0xffff  // 7-white
        };
        if (v == 0)
        { // all attributes off
          if (nVals == 1)
          {
            fg = ILI9341_WHITE;
            bg = FAKEBLACK;
          }
          bold = 0;
        }
        else if (v == 1)
        { // all attributes off
          bold = 1;
        }
        else if (v >= 30 && v < 38)
        { // fg colors
          fg = colors[v - 30];
        }
        else if (v >= 40 && v < 48)
        {
          bg = colors[v - 40];
        }
      }
      vals[0] = vals[1] = vals[2] = vals[3] = 0;
      nVals = 0;
    }
    else
    {
      escMode = 0;
      vals[0] = vals[1] = vals[2] = vals[3] = 0;
      nVals = 0;
    }
    return;
  }
  if (c == 10)
  {
    scroll();
    return;
  }
  if (c == 13)
  {
    xp = 0;
    return;
  }
  if (c == 8)
  {
    if (xp > 0)
      xp -= charWd * sx;
    M5.Lcd.fillRect(xp, yp, charWd * sx, charHt * sy, FAKEBLACK);
    return;
  }
  if (xp < screenWd)
    drawChar(xp, yp, c, fg, bg, sx, sy);
  xp += charWd * sx;
  if (xp >= screenWd && wrap)
    scroll();
}

void printString(char *str)
{
  while (*str)
    printChar(*str++);
}

void setupScroll(uint16_t tfa, uint16_t bfa)
{
  M5.Lcd.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
  M5.Lcd.writedata(tfa >> 8);
  M5.Lcd.writedata(tfa);
  M5.Lcd.writedata((240 - tfa - bfa) >> 8);
  M5.Lcd.writedata(240 - tfa - bfa);
  M5.Lcd.writedata(bfa >> 8);
  M5.Lcd.writedata(bfa);
}

void scrollFrame(uint16_t vsp)
{
  M5.Lcd.writecommand(ILI9341_VSCRSADD); // Vertical scrolling start address
  M5.Lcd.writedata(vsp >> 8);
  M5.Lcd.writedata(vsp);
}



void tty_cls(){
    M5.Lcd.clear(FAKEBLACK);
    M5.Lcd.setCursor(0, 0);
    xp = 0;
    yp = 0;
    scrollFrame(0);
}

void tty_setup(){
  //setupScrollArea(0, 0);
  setupScroll(TOP_FIXED_AREA, BOT_FIXED_AREA);
  tty_cls();
  scrollMode = 0;
  printString((char*)"\e\n[0;44m (VT100 tty_ready!) \e[0m\n");
  delay(200);
  tty_cls();
  scrollMode = 0;
}

int aprintf(char *str, ...)
{
  int i, j, count = 0;

  va_list argv;
  va_start(argv, str);
  for (i = 0, j = 0; str[i] != '\0'; i++)
  {
    if (str[i] == '%')
    {
      count++;

      Serial.write(reinterpret_cast<const uint8_t *>(str + j), i - j);

      switch (str[++i])
      {
      case 'd':
        Serial.print(va_arg(argv, int));
        break;
      case 'l':
        Serial.print(va_arg(argv, long));
        break;
      case 'f':
        Serial.print(va_arg(argv, double));
        break;
      case 'c':
        Serial.print((char)va_arg(argv, int));
        break;
      case 's':
        Serial.print(va_arg(argv, char *));
        break;
      case '%':
        Serial.print("%");
        break;
      default:;
      };

      j = i + 1;
    }
  };
  va_end(argv);

  if (i > j)
  {
    Serial.write(reinterpret_cast<const uint8_t *>(str + j), i - j);
  }

  return count;
}
