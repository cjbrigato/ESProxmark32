#include <M5OnScreenKeyboard.h>
#include <M5ButtonDrawer.h>
#include <M5StackUpdater.h>
#include "MHeader.h"
#include "common/common_firmware.h"
#include "authorsAssets.h"



TFT_eSprite anim = TFT_eSprite(&M5.Lcd);


void fadeUp(uint32_t ms) {
  for (int i = 0; i < 90; i++) {
    M5.lcd.setBrightness(i);
    delay(ms);
  }
}

void fadeDown(uint32_t ms) {
  for (int i = 89; i >= 0; i--) {
    M5.lcd.setBrightness(i);
    delay(ms);
  }
  M5.lcd.setBrightness(0);
}

void TtySetupAnim(){
  fadeUp(3);
  delay(100);
  for (int i = M5.Lcd.width(); i>=0;i--){
     M5.Lcd.drawFastHLine(i, 8, 1, 0xC618);
     M5.Lcd.drawFastHLine(320-i, 240 - 42, 1, 0xC618);
     delay(400/M5.Lcd.width());
  }
  for (int i = 0; i < 8; i++){
    M5.Lcd.fillRect(0, i, 320, 1, 0x0000);
    M5.Lcd.fillRect(0, 240-(i*(40/8)), 320, 1*(40/8), 0x0000);
    delay(400/8);
  }
  delay(100);
  M5.Lcd.fillRect(0, 240 - 40, 320, 40, 0x0000);
}
void PrintMOTD(){

  printString("\r\n");
  printString(_RED_("  ____  __  __ _____\r\n"));
  printString(_RED_("|  _ \\|  \\/  |___ /\r\n"));
  printString(_RED_("| |_) | |\\/| | |_ \\  ")_YELLOW_("RDV4.0\r\n"));
  printString(_RED_("|  __/| |  | |___) |") " ------\r\n");
  printString(_RED_(" |_|   |_|  |_|____/  ")_YELLOW_("vESP32\r\n"));
  printString("==================\r\n");
  printString("> Everything is ready\r\n");
  printString("> Click [START] when the pm3 BTaddon\r\n");
  printString("> has stoped blinking\r\n");
  printString("~\r\n");

}
void Intropyrights(){
  M5.Lcd.fillScreen(TFT_WHITE);
  ///////////////////////////////// INTRO SECTION////////////////////////////////
  fadeUp(3);
  int stepper=8;
  for(int i = M5.Lcd.width();i>(M5.Lcd.width()-150)/2;i--){
    M5.Lcd.pushImage(i,20,150,66,(const uint16_t*)rrglogo_white_150x66_bin);
    M5.Lcd.fillRect(i+150, 20-2,stepper+4, 66+2, TFT_WHITE);
    i-=stepper;
  }
  delay(200);
  for(int i = M5.Lcd.width();i>(M5.Lcd.width()-81)/2;i--){
    M5.Lcd.pushImage(i,100,81,109,(const uint16_t*)icemanlogo_white__81x109_bin);
    M5.Lcd.fillRect(i+81+1, 100-1,stepper+2, 109+2, TFT_WHITE);
    i-=stepper;
  }
  delay(2000);
  stepper=8;
    for(int i = (M5.Lcd.width()-150)/2;i>(-150);i--){
    M5.Lcd.pushImage(i,20,150,66,(const uint16_t*)rrglogo_white_150x66_bin);
    M5.Lcd.fillRect(i+150, 20-2,stepper+4, 66+2, TFT_WHITE);
    i-=stepper;
  }
  for(int i = (M5.Lcd.width()-81)/2;i>(-81);i--){
    M5.Lcd.pushImage(i,100,81,109,(const uint16_t*)icemanlogo_white__81x109_bin);
    M5.Lcd.fillRect(i+81+1, 100-1,stepper+2, 109+2, TFT_WHITE);
    i-=stepper;
  }

    for(int i = M5.Lcd.width();i>(M5.Lcd.width()-150)/2;i--){
    M5.Lcd.pushImage(i,(220-180)/2,150,180,(const uint16_t*)rdv40logo_white_150x180_bin);
    //M5.Lcd.fillRect(i+150, 20-2,stepper+4, 66+2, TFT_WHITE);
    i-=stepper;
  }
  delay(2000);
  fadeDown(3);
  //transition
  delay(10);
  for (int i = M5.Lcd.width(); i>=0;i--){
     M5.Lcd.fillRect(i,0, 1, M5.Lcd.height(), FAKEBLACK);
     delay(400/M5.Lcd.width());
  }
  }

int *animation_parse_size(const char *path) {
  static int res[2];
  int width;
  int height;
  char filename[32 + 1];
  sprintf(filename, "%s", path);
  char *psize = strtok(filename, "_");
  psize = strtok(NULL, "_");
  char *separator = strchr(psize, 'x');

  if (separator != 0) {
    *separator = 0;
    width = atoi(psize);
    ++separator;
    height = atoi(separator);
  }

  res[0] = width;
  res[1] = height;
  return res;
}

// beware that SIZE is now deduced from filename , eg loader.bin_149x88 makes
// widht 149 and height 88. Will PANIC if not formated correctly on SD or SPIFFS)
// void animate(uint16_t w, uint16_t h, char * path, uint8_t FromSPIFFS,
// uint16_t x, uint16_t y, int ms, uint16_t transparent_color){
void animate(char *path, uint8_t FromDIRENT, uint8_t FromSPIFFS, uint16_t x,
             uint16_t y, uint8_t FromCenter, int ms,
             uint16_t transparent_color) {
  File root;
  File file;
  if (!FromSPIFFS) {
    if (FromDIRENT) {
      root = SD.open(path);
      file = root.openNextFile();
    } else {
      file = SD.open(path);
    }
  } else {
    SPIFFS.begin();
    if (FromDIRENT) {
      root = SPIFFS.open(path);
      file = root.openNextFile();
    } else {
      file = SPIFFS.open(path);
    }
  }

  Serial.printf("Filename : %s\n", file.name());

  int *animsize = animation_parse_size(file.name());
  int width = *(animsize);
  int height = *(animsize + 1);

  if (FromCenter) {
    x = (320 - width) / 2;
    y = (240 - height) / 2;
  }

  uint16_t **picture = (uint16_t **)ps_malloc(width * sizeof(uint16_t *));
  for (int i = 0; i < width; i++) {
    picture[i] = (uint16_t *)ps_malloc(height * sizeof(uint16_t));
  }

  anim.createSprite(width, height);
  anim.setSwapBytes(true);

  while (file.available()) {
    file.read((uint8_t *)picture, width * height * 2);
    anim.pushImage(0, 0, width, height, (uint16_t *)picture);
    anim.pushSprite(x, y, transparent_color);
    delay(ms);
  }
  file.close();
  // TODO
  /*  for (int i=0;i<w;i++){
      free(picture[i]);
    }
    free(picture);*/
}