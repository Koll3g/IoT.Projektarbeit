#include "ValveFsm.h"

#include <../lib/fsm/fsm.h>

#include <Arduino.h>

typedef struct sValveContext {
    LED *leds; //Indicating status of valve -> green = open | red = closed | blue = undefined
    bool buttonOn;
    bool valveOpen; //Assume valve would move according to this bool -> true = open
    bool valveInitialized; 
    uint TargetMoistureValue = 50; //default value if cloud is not available - basic irrigation ensured
    uint ActualMoistureValue = 50; //default value, in case soil moisture sensor is offline  we do not want to flood the field
    QwiicButton *button;
}tValveContext;

typedef enum eValveState {
    VALVE_ST_UNDEF = 0,
    VALVE_ST_CLOSED,
    VALVE_ST_OPEN,
    
    VALVE_ST_COUNT
}tValveState;

static void ButtonValveToggle(tValveContext *me);

static FSM_STATE_HANDLER(Undef);
static FSM_STATE_HANDLER(Closed);
static FSM_STATE_HANDLER(Open);

static const tFSM_State ValveChanger[] = {
    FSM_STATE_DESCRIBE("Water Valve undefined", Undef),
    FSM_STATE_DESCRIBE("Water Valve closed", Closed),
    FSM_STATE_DESCRIBE("Water Valve open", Open),

    FSM_STATE_LAST()
};

static tValveContext ValveChangerContext;
static FSM *ValveChangerInstance;

void ValveChangerInit(LED *leds, QwiicButton *button) {
    ValveChangerContext.leds = leds;
    ValveChangerContext.button = button;
    ValveChangerInstance = new FSM(VALVE_ST_UNDEF, ValveChanger, &ValveChangerContext);
}

int  ValveChangerRun(void) {
    return ValveChangerInstance->run();
}

static FSM_STATE_HANDLER(Undef) {
    tValveContext *me = (tValveContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering undefined Valve state - initializing");
        me->leds->setLEDColor(0,0,255);
        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {
        
        //do something to initialize valve sub-system
        me->valveInitialized = true;
        Serial.println("Valve initialized");

        //as a first step immediately close valve
        if(me->valveInitialized){
            fsm->NextStateSet(VALVE_ST_CLOSED);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting undefined Valve state");
    }
    return 0;
}

static FSM_STATE_HANDLER(Closed) {
    tValveContext *me = (tValveContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering closed Valve state");
        
        //Set LED and valve state to closed
        me->leds->setLEDColor(255, 0, 0);
        me->valveOpen = false;

        delay(2000);

        Serial.println("Valve closed");
    }
    else if (reason == FSM_REASON_DO) {
        
        //wait for button to be pressed to manually open the valve
        if (me->button->isPressed()) {
            fsm->NextStateSet(VALVE_ST_OPEN);
            ButtonValveToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting closed Valve state");
    }
    return 0;
}

static FSM_STATE_HANDLER(Open) {
    tValveContext *me = (tValveContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering open Valve state");
        
        //Set LED and valve state to open
        me->leds->setLEDColor(0, 255, 0);
        me->valveOpen = true;

        delay(2000);

        Serial.println("Valve open");
    }
    else if (reason == FSM_REASON_DO) {
        
        //wait for button to be pressed to manually close the valve
        if (me->button->isPressed()) {
            fsm->NextStateSet(VALVE_ST_CLOSED);
            ButtonValveToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting open Valve state");
    }
    return 0;
}

static void ButtonValveToggle(tValveContext *me) {
  if (!me->buttonOn) {
    me->button->LEDconfig(1, 0, 0);
  }
  else {
    me->button->LEDconfig(0, 0, 0);
  }
  me->buttonOn = !me->buttonOn;
}

void SetTargetMoistureValue(uint targetMoisture){
    ValveChangerContext.TargetMoistureValue = targetMoisture;
}
void SetActualMoistureValue(uint actualMoisture){
    ValveChangerContext.ActualMoistureValue = actualMoisture;
}