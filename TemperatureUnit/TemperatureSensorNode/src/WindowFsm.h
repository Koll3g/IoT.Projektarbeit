#pragma once

#include <../lib/fsm/fsm.h>
#include <../lib/led/QwiicLedStick.h>
#include <../lib/button/Sparkfun_Qwiic_Button.h>

void WindowChangerInit(LED *leds, QwiicButton *button);
int  WindowChangerRun(void);

//Values written by mqtt 
//uint TargetMoistureValue = 50; //default value if cloud is not available - basic irrigation ensured
//uint ActualMoistureValue = 50; //default value, in case soil moisture sensor is offline  we do not want to flood the field

void SetTargetTempWindow(uint targetTemperature);
void SetActualInteriorTempWindow(uint actualInteriorTemperature);
void SetActualExteriorTempWindow(uint actualExteriorTemperature);

