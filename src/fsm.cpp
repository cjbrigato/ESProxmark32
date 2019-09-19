
#include <M5OnScreenKeyboard.h>
//#include <M5JoyStick.h>
#include <M5ButtonDrawer.h>
#include <M5StackUpdater.h>
#include "MHeader.h"
#include "rdv40App.h"
#include "common/common_firmware.h"
#include "fsm.h"

int msecHold = 200;
bool swapBtnBC;

extern M5ButtonDrawer stdButtonDrawer;
eSystemState eNextState = Appmain_State;

int haschanged = 0;
int appshown = 0;

eSystemState Appmain_StateHandler()
{
  /*if (M5.BtnA.isPressed())
  {
    stdButtonDrawer.setText(0, M5.BtnA.pressedFor(msecHold) ? "" : "");
  }
  else
  {*/
    stdButtonDrawer.setText(0, "");
  //}
  stdButtonDrawer.setText(swapBtnBC ? 1 : 2, "");
  stdButtonDrawer.setText(swapBtnBC ? 2 : 1, "[START]");

  stdButtonDrawer.draw(false);

  if (M5.BtnB.wasPressed())
  {
    stdButtonDrawer.setText(swapBtnBC ? 2 : 1, "");
    SendCommandNG(CMD_STANDALONE, NULL, 0);
    haschanged = 1;
    appshown = 1;
    //delay(500); //debunking button
    return ESP32Appmain_State;
  }
  return Appmain_State;
}

char next[20];
char prev[20];
int nextindex;
int previndex;

static void drawAppButton(){
  int iconX = (320 / 2) - 8;
  int iconY = 240 - 34;
  //  char next[20];
//  char prev[20];
//  int
  nextindex=appshown+1;
//  int
  previndex=appshown-1;
  if ((nextindex) == totalApps){
     nextindex=0;
  }
  if ((previndex) < 0){
     previndex=totalApps - 1;
  }

  if (haschanged)
  {
    printString("\r");
    M5.Lcd.drawLine(0, 240 - 42, 320, 240 - 42, 0xC618);
    M5.Lcd.fillRect(0, iconY, 320, 40, TFT_BLACK);
    M5.Lcd.fillRect(0, 240 - 40, 320, 40, 0x0000); // 0x630C);
    //M5.Lcd.drawJpg(Apps[appshown].icon,Apps[appshown].iconlen,iconX,iconY,320-iconX,240-iconY,0,0,JPEG_DIV_NONE);
    M5.Lcd.setCursor((340 / 2) + 16, 240 - 30);
    //M5.Lcd.printf("%s",Apps[appshown].name);
    //M5.Lcd.setTextDatum(BC_DATUM);
    String appname = String('[' + String(Apps[appshown].name) + ']');
    //M5.Lcd.drawString(appname,160,220);
    //stdButtonDrawer.setText(swapBtnBC ? 2 : 1, "");
    stdButtonDrawer.draw(false);
    //M5.Lcd.setCursor((340/2)+16,240-30);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(appname, 160, 232);
    M5.Lcd.drawString("<<", 100,iconY+16);
    M5.Lcd.drawString(">>", 220,iconY+16);
    M5.Lcd.drawJpg(Apps[appshown].icon, Apps[appshown].iconlen, iconX, iconY, 320 - iconX, 240 - iconY, 0, 0, JPEG_DIV_NONE);
    M5.Lcd.drawJpg(Apps[previndex].icon,Apps[previndex].iconlen, iconX/3,iconY+4,320 - (iconX/3), 240 - (iconY+4), 0, 0, JPEG_DIV_NONE); 
    M5.Lcd.drawJpg(Apps[nextindex].icon,Apps[nextindex].iconlen, (iconX/3)*5,iconY+4,320 - (5*(iconX/3)), 240 - (iconY+4), 0, 0, JPEG_DIV_NONE);
    
    haschanged = 0;
  }
}

eSystemState ESP32Appmain_StateHandler()
{



  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  //sprintf(next,">> (%s)",Apps[nextindex].name);
  //sprintf(prev,"(%s) <<",Apps[previndex].name);
  //stdButtonDrawer.setText(0, prev);
  //stdButtonDrawer.setText(swapBtnBC ? 1 : 2, next);
  drawAppButton();
  if (M5.BtnA.wasPressed())
  {
    if (appshown == 0)
    {
      appshown = (totalApps - 1);
    }
    else
    {
      appshown--;
    }
    haschanged = 1;
    drawAppButton();
    return ESP32Appmain_State;
  }
  if (M5.BtnC.wasPressed())
  {
    appshown++;
    if (appshown == totalApps)
      appshown = 0;
    haschanged = 1;
    drawAppButton();
    return ESP32Appmain_State;
  }
  if (M5.BtnB.wasPressed())
  {
    SendCommandNG(Apps[appshown].cmd, NULL, 0);
    haschanged = 1;
    drawAppButton();
    if (appshown == 0)
    {
      fadeDown(3);
      tty_setup();
      tty_cls();
      fadeUp(3);
      TtySetupAnim();
      PrintMOTD();
      return Appmain_State;
    }
  }

  return ESP32Appmain_State;
}
