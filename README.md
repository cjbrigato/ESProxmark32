# ESProxmark32 (Demoà
## ESP32Base Working client for BTaddon esp32
This is very very alpha version wich will only demonstrate COMMS and possibilitis of the device
Still, it's enough to implement anything against appmain() or a custom standalone, so that's why this is provided 90% downstriped.
## Needs :
* HC06/HC05 setuped as slave of btaddon (ROLE=M, +NAME%SLAVEDEVICENAME)
* May need specific Standalone mode (HF_ESP32) depending on your implementation (specific IHM'ed - and that's the very point of such device/client - behavior WILL indeed need, even if for the demo it doesn't) but if you just need demonstration of COMMS it mayn ot.
* May need ESP32 with PSRAM support depending on your implementation
## Setup
TODO
# Next
TODO
# Not included
* Wifi "backdoor"/MITM part, as it complicates too much things so this demo loose it's purpose
* Incoming full firmware to be released via RRG (Reprensent ~90% much more of provided but still makes this example loose it's purpose)
# Included
* Example base corresponding standalone for pm3side (launched when [START] is clicked
* SelfUpdater via SPIFFS / OTA
* SDcard FTP Server (needs wifi working on wifiOPS.c
* Compatibility with M5Stack-SDUpdater
* Platformio/Arduino Version for Easy accessibility. Should also work with ArduinioESP32 as an esp-idf component with proper sdkconfig.h (example in provided one)

