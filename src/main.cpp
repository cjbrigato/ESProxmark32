#include "common/common_firmware.h"
#include "selfupdater.h"

#include <M5ButtonDrawer.h>
#include <M5OnScreenKeyboard.h>
#include <M5StackUpdater.h>
#include "MHeader.h"
#include "fsm.h"

#include "SDFTPServer/CBFTPserver.h"
#include "SDFTPServer/MenuCallBack.h"


MHeader header;

M5ButtonDrawer stdButtonDrawer;

TaskHandle_t ReceiverTask_handle;
TaskHandle_t AppTask_handle;

byte dataBuffer = 0;
uint8_t loopcounter = 0;
extern eSystemState eNextState;

extern int yp;

template <class T>
void callBackExec(MenuItem *sender) {
  T menucallback;
  menucallback(sender);
}

void AppTask(void *pvParameters) {
  for (;;) {
    M5.update();
    header.draw();
    M5.Lcd.drawLine(0, 240 - 42, 320, 240 - 42, 0xC618);

    switch (eNextState) {
      case Appmain_State:
        eNextState = Appmain_StateHandler();
        break;
      case ESP32Appmain_State:
        eNextState = ESP32Appmain_StateHandler();
        break;
    }
    delay(100);
  }
}

void ReceiverTask(void *pvParameters) {
  for (;;) {
    while (Serial2.available()) {
      receive_pm3cmd();
    }
    delay(10);
  }
}

void setup() {
  M5.begin();
  M5.Speaker.begin();
  M5.Speaker.mute();

  Wire.begin();
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  if (digitalRead(BUTTON_B_PIN) == 0) {
    Serial.println("Self Update Operation.");
    SelfUpdate();
  }

  if (digitalRead(BUTTON_C_PIN) == 0) {
    Serial.println("Launching SDFTPServer");
    wifiMulti_setup();
    // wifi_setup();
    MenuItem *ftpserver =
        new MenuItem("FTP Server (SPIFFS)", callBackExec<CBFTPserverSD>);
    delay(1000);
    callBackExec<CBFTPserverSD>(ftpserver);
  }

  WiFi.mode(WIFI_OFF);
  btStop();

  Intropyrights();
  M5.Lcd.setTextColor(TFT_WHITE, FAKEBLACK);
  tty_setup();
  tty_cls();

  TtySetupAnim();

  xTaskCreatePinnedToCore(AppTask, "AppTask", 1024 * 32, NULL, 2,
                          &AppTask_handle, 1);

  Serial2.begin(115200, SERIAL_8N1, 36, 26);
  Serial2.setRxBufferSize(16 * 1024);

  xTaskCreatePinnedToCore(ReceiverTask, "ReceiverTask", 1024 * 32, NULL, 3,
                          &ReceiverTask_handle, 1);

  Serial.printf("End of setup\r\n");
  PrintMOTD();
  
}

void loop(void) { delay(1000); }
