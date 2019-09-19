#ifndef RDV40_APP_H
#define RDV40_APP_H
#include "common/common_firmware.h"
#include "common/hf_colin_esp32_cmd.h"
#include "rdv40AppAssets.h"

typedef struct
{
    uint8_t id;
    const char *name;
    uint16_t cmd;
    const uint8_t *icon;
    const uint32_t iconlen;
} RDV40_App;

const int totalApps = 4;
RDV40_App spiffs_tree = {.id = 1, .name = "Spiffs Tree", .cmd = CMD_SPIFFS_PRINT_TREE, .icon = spiffs_tree_jpg, .iconlen = spiffs_tree_jpg_len};
RDV40_App quit = {.id = 0, .name = "Quit", .cmd = ESP32CMD_QUIT, .icon = quit_jpg, .iconlen = quit_jpg_len};
RDV40_App status = {.id = 2, .name = "Status", .cmd = CMD_STATUS, .icon = status_jpg, .iconlen = status_jpg_len};
RDV40_App hf_colin = {.id = 3, .name = "HF_COLIN", .cmd = ESP32CMD_HFCOLIN, .icon = hf_colin_jpg, .iconlen = hf_colin_jpg_len};
RDV40_App Apps[totalApps] = {quit, spiffs_tree, status, hf_colin};

#endif