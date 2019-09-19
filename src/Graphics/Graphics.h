#ifndef _GRAPHICS_H
#define _GRAPHICS_H

void Intropyrights();
void TtySetupAnim();
void fadeUp(uint32_t ms);
void fadeDown(uint32_t ms);
void PrintMOTD();
int *animation_parse_size(const char *path);
void animate(char *path, uint8_t FromDIRENT, uint8_t FromSPIFFS, uint16_t x,
             uint16_t y, uint8_t FromCenter, int ms,
             uint16_t transparent_color);

#endif