#pragma once

#include <../lib/fsm/fsm.h>
#include <../lib/led/QwiicLedStick.h>
#include <../lib/button/Sparkfun_Qwiic_Button.h>

void ValveChangerInit(LED *leds, QwiicButton *button);
int  ValveChangerRun(void);

void SetIrrigationTargetValue(int targetValue);
void SetIrrigationActualValue(int actualValue);