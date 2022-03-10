#pragma once

#include <../lib/fsm/fsm.h>
#include <../lib/led/QwiicLedStick.h>
#include <../lib/button/Sparkfun_Qwiic_Button.h>

void TempSimChangerInit(LED *leds, QwiicButton *button);
int  TempSimChangerRun(void);

