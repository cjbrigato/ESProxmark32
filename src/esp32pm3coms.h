#ifndef __ESP32_PM4_COMS_H__
#define __ESP32_PM4_COMS_H__



PacketResponseNG receive_pm3cmd(void);
int reply_ng(uint16_t cmd, int16_t status, uint8_t *data, size_t len);
void SendCommandNG(uint16_t cmd, uint8_t *data, size_t len);
#endif