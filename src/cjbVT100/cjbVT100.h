#ifndef __CJB_VT100_H__
#define __CJB_VT100_H__

//from aprintf.cpp
int aprintf(char *str, ...);
void tty_setup();
void tty_cls();
void setupScrollArea(uint16_t tfa, uint16_t bfa);
void printString(char *str);
void scrollFrame(uint16_t vsp);
void printChar(char c);
void scrollAddress(uint16_t vsp);
int get_scroll_mode();

#define _BLUE_(s) "\x1b[34m" s "\x1b[0m "
#define _RED_(s) "\x1b[31m" s "\x1b[0m "
#define _GREEN_(s) "\x1b[32m" s "\x1b[0m "
#define _YELLOW_(s) "\x1b[33m" s "\x1b[0m "
#define _MAGENTA_(s) "\x1b[35m" s "\x1b[0m "
#define _CYAN_(s) "\x1b[36m" s "\x1b[0m "
#define FAKEBLACK 0x10E6

#endif