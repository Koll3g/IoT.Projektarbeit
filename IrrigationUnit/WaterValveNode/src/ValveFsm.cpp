#include "ValveFsm.h"

#include <../lib/fsm/fsm.h>

#include <Arduino.h>

typedef struct sValveContext {
    LED *leds; //Indicating status of valve -> green = open | red = closed | blue = undefined
    bool buttonOn;
    bool valveOpen; //Assume valve would move according to this bool -> true = open
    bool valveInitialized; 
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

static tValveContext ValveChangerContext = { 0 };
static FSM *ValveChangerInstance;

void ValveChangerInit(LED *leds, QwiicButton *button) {
    ValveChangerContext.leds = leds;
    ValveChangerContext.button = button;
    ValveChangerInstance = new FSM(VALVE_ST_UNDEF, ValveChanger, &ValveChangerContext);
}

int  ColorChangerRun(void) {
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
    }
    else if (reason == FSM_REASON_DO) {
        
        Serial.println("Valve closed");

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

static FSM_STATE_HANDLER(Blue) {
    tValveContext *me = (tValveContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering Blue");
        me->leds->setLEDColor(0, 0, 255);
        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {
        if (me->button->isPressed()) {
            fsm->NextStateSet(COLOR_CHANGER_ST_RED);
            ButtonLedToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting Blue");
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