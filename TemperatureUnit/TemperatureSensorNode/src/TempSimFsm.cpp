#include "TempSimFsm.h"

#include <../lib/fsm/fsm.h>

#include <Arduino.h>

#include "main.h"

typedef struct sTempSimContext {
    LED *leds; //Indicating status of TempSim -> green = open | red = closed | blue = undefined
    bool buttonOn;
    QwiicButton *button;
}tTempSimContext;

typedef enum eTempSimState {
    TEMPSIM_ST_NORMAL = 0,
    TEMPSIM_ST_HOT,
    TEMPSIM_ST_COLD,
    
    TEMPSIM_ST_COUNT
}tTempSimState;

static void ButtonTempSimToggle(tTempSimContext *me);

static FSM_STATE_HANDLER(Normal);
static FSM_STATE_HANDLER(Hot);
static FSM_STATE_HANDLER(Cold);

static const tFSM_State TempSimChanger[] = {
    FSM_STATE_DESCRIBE("Temperature 20°C (normal)", Normal),
    FSM_STATE_DESCRIBE("Temperature 25°C", Hot),
    FSM_STATE_DESCRIBE("Temperature 15°C", Cold),

    FSM_STATE_LAST()
};

static tTempSimContext TempSimChangerContext;
static FSM *TempSimChangerInstance;

void TempSimChangerInit(LED *leds, QwiicButton *button) {
    TempSimChangerContext.leds = leds;
    TempSimChangerContext.button = button;
    TempSimChangerInstance = new FSM(TEMPSIM_ST_NORMAL, TempSimChanger, &TempSimChangerContext);
}

int  TempSimChangerRun(void) {
    return TempSimChangerInstance->run();
}

static FSM_STATE_HANDLER(Normal) {
    tTempSimContext *me = (tTempSimContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering normal TempSim State");
        me->leds->setLEDColor(0,255,0);
        MqttUpdateTempSimState("20");
        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {

        //wait for button to be pressed to switch to next temperature
        if (me->button->isPressed()) {
            fsm->NextStateSet(TEMPSIM_ST_HOT);
            ButtonTempSimToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting normal TempSim state");
    }
    return 0;
}

static FSM_STATE_HANDLER(Hot) {
    tTempSimContext *me = (tTempSimContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering hot TempSim state");
        me->leds->setLEDColor(255, 0, 0);
        MqttUpdateTempSimState("25");
        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {

        //wait for button to be pressed to switch to next temperature
        if (me->button->isPressed()) {
            fsm->NextStateSet(TEMPSIM_ST_COLD);
            ButtonTempSimToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting hot TempSim state");
    }
    return 0;
}

static FSM_STATE_HANDLER(Cold) {
    tTempSimContext *me = (tTempSimContext *)context;

    if (reason == FSM_REASON_ENTER) {
        Serial.println("Entering cold TempSim state");
        me->leds->setLEDColor(0, 0, 255);
        MqttUpdateTempSimState("15");
        delay(2000);
    }
    else if (reason == FSM_REASON_DO) {

        //wait for button to be pressed to switch to next temperature
        if (me->button->isPressed()) {
            fsm->NextStateSet(TEMPSIM_ST_NORMAL);
            ButtonTempSimToggle(me);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        Serial.println("Exitting cold TempSim state");
    }
    return 0;
}

static void ButtonTempSimToggle(tTempSimContext *me) {
  if (!me->buttonOn) {
    me->button->LEDconfig(1, 0, 0);
  }
  else {
    me->button->LEDconfig(0, 0, 0);
  }
  me->buttonOn = !me->buttonOn;
}


