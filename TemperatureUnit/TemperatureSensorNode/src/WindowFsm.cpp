#include "WindowFsm.h"

#include <../lib/fsm/fsm.h>

#include <Arduino.h>

#include "main.h"

typedef struct sWindowContext {
    LED *leds; //Indicating status of Window -> green = open | red = closed | blue = undefined
    bool buttonOn;
    bool WindowOpen; //Assume Window would move according to this bool -> true = open
    bool WindowInitialized; 
    uint TargetTempValue = 25; //default value if cloud is not available
    uint ActualInteriorTempValue = 25; //default value
    uint ActualExteriorTempValue = 25; //default value
    QwiicButton *button;
}tWindowContext;

typedef enum eWindowState {
    WINDOW_ST_UNDEF = 0,
    WINDOW_ST_CLOSED,
    WINDOW_ST_OPEN,
    
    WINDOW_ST_COUNT
}tWindowState;

static void ButtonWindowToggle(tWindowContext *me);

static FSM_STATE_HANDLER(Undef);
static FSM_STATE_HANDLER(Closed);
static FSM_STATE_HANDLER(Open);

static const tFSM_State WindowChanger[] = {
    FSM_STATE_DESCRIBE("Window Position undefined", Undef),
    FSM_STATE_DESCRIBE("Window closed", Closed),
    FSM_STATE_DESCRIBE("Window open", Open),

    FSM_STATE_LAST()
};

static tWindowContext WindowChangerContext;
static FSM *WindowChangerInstance;

void WindowChangerInit(LED *leds, QwiicButton *button) {
    WindowChangerContext.leds = leds;
    WindowChangerContext.button = button;
    WindowChangerInstance = new FSM(WINDOW_ST_UNDEF, WindowChanger, &WindowChangerContext);
}

int  WindowChangerRun(void) {
    return WindowChangerInstance->run();
}

static FSM_STATE_HANDLER(Undef) {
    tWindowContext *me = (tWindowContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering undefined Window state - initializing");
        me->leds->setLEDColor(0,0,255);
        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {
        
        //do something to initialize Window sub-system
        me->WindowInitialized = true;
        Serial.println("Window initialized");

        //as a first step immediately close Window
        if(me->WindowInitialized){
            fsm->NextStateSet(WINDOW_ST_CLOSED);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting undefined Window state");
    }
    return 0;
}

static FSM_STATE_HANDLER(Closed) {
    tWindowContext *me = (tWindowContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering closed Window state");
        
        //Set LED and Window state to closed
        me->leds->setLEDColor(255, 0, 0);
        me->WindowOpen = false;
        MqttUpdateWindowState("closed");
        Serial.println("Window closed");

        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {
        
        //Check inside temp vs. outside temp
        if(me->TargetTempValue < me->ActualInteriorTempValue){
            //Make sure outside is not even warmer than inside
            if(me->ActualInteriorTempValue < me->ActualExteriorTempValue){
                Serial.println("Inside temperature above target, but outside is even warmer - abort window opening");
            }
            else{
                Serial.println("Inside temperature above target temperature - open windows as outside is cooler");
                fsm->NextStateSet(WINDOW_ST_OPEN);
            }
        }
        //wait for button to be pressed to manually open the Window
        else if (me->button->isPressed()) {
            fsm->NextStateSet(WINDOW_ST_OPEN);
            ButtonWindowToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting closed Window state");
    }
    return 0;
}

static FSM_STATE_HANDLER(Open) {
    tWindowContext *me = (tWindowContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering open Window state");
        
        //Set LED and Window state to open
        me->leds->setLEDColor(0, 255, 0);
        me->WindowOpen = true;

        MqttUpdateWindowState("open");
        Serial.println("Window open");

        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {
        //Thought about case where outside is becoming warmer than inside -> should we close the windows again? 
        //Problem: this would create an endless loop as we don't have any other measure to cool beside opening the windows

        //Check inside temp vs. outside temp
        if(me->TargetTempValue >= me->ActualInteriorTempValue){
            Serial.println("Inside temperature at or below target temperature - close windows");
            fsm->NextStateSet(WINDOW_ST_CLOSED);
        }
        //wait for button to be pressed to manually open the Window
        else if (me->button->isPressed()) {
            fsm->NextStateSet(WINDOW_ST_CLOSED);
            ButtonWindowToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting open Window state");
    }
    return 0;
}

static void ButtonWindowToggle(tWindowContext *me) {
  if (!me->buttonOn) {
    me->button->LEDconfig(1, 0, 0);
  }
  else {
    me->button->LEDconfig(0, 0, 0);
  }
  me->buttonOn = !me->buttonOn;
}

void SetTargetTempWindow(uint targetTemperature){
    WindowChangerContext.TargetTempValue = targetTemperature;
}
void SetActualInteriorTempWindow(uint actualInteriorTemperature){
    WindowChangerContext.ActualInteriorTempValue = actualInteriorTemperature;
}
void SetActualExteriorTempWindow(uint actualExteriorTemperature){
    WindowChangerContext.ActualExteriorTempValue = actualExteriorTemperature;
}
