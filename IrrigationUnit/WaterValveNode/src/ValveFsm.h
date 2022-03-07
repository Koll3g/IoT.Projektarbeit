#pragma once

#include <../lib/fsm/fsm.h>
#include <../lib/led/QwiicLedStick.h>
#include <../lib/button/Sparkfun_Qwiic_Button.h>

void ColorChangerInit(LED *leds, QwiicButton *button);
int  ColorChangerRun(void);