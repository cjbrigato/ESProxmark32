#include "common/common_firmware.h"

#include <WiFi.h>
#include <WiFiMulti.h>

#include "wifiOps.h"

//Wifi should've been handled beforehand 
const char *ssid = "xxxx";
const char *password = "xxxxx";

WiFiMulti wifiMulti;    

void wifiMulti_setup(){
    wifiMulti.addAP(ssid, password);
    wifiMulti.addAP("xxxxx", "xxxx");

      for (int loops = 40; loops > 0; loops--)
  {
    if (wifiMulti.run() == WL_CONNECTED)
    {
      Serial.println("");
      Serial.print("WiFi connected ");

      Serial.print("IP address: ");

      Serial.println(WiFi.localIP());

      break;
    }
    else
    {
      Serial.println(loops);
      delay(10);
    }
  }

  if (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("WiFi connect failed");
    //delay(1000);
    //ESP.restart();
  }

  Serial.print("Ready! Ipbtw ");
  Serial.print(WiFi.localIP());

}

void wifi_setup(){

    if (WiFi.status() != WL_CONNECTED) {
      WiFi.mode(WIFI_MODE_STA);
      WiFi.begin();
      //M5.Lcd.drawString("WiFi waiting...", 10, 60, 1);
    }

}
