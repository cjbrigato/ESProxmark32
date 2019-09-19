#ifndef _FSM_H
#define _FSM_H

typedef enum
{
  Appmain_State,
  ESP32Appmain_State,
} eSystemState;
eSystemState Appmain_StateHandler();
eSystemState ESP32Appmain_StateHandler();

#endif