#ifndef HF_COLIN_ESP32_APPMAIN_H
#define HF_COLIN_ESP32_APPMAIN_H

#include "common.h"
#include "pm3_cmd.h"
#include "cmd.h"
#include "hf_colin_esp32_cmd.h"
#include "appmain.h"

int ESP32PacketReceived(PacketCommandNG *packet);

#endif
