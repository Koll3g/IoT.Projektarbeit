#include <Arduino.h>

#include <WiFi/WiFiManager.h>
#include <MQTT/MqttClient.h>
#include <StopWatch.h>

#include <Sparkfun_Qwiic_Button.h>
#include <QwiicLedStick.h>

#include <cstring>

#include <config/secrets.h>
#include <config/mqtt.h>

#include <Path.h>

#include "LedUtils.h"

#include "soilSens/QwiicSoilMoistureSensor.h"

using namespace ZbW;
using namespace ZbW::CommSubsystem;

static bool QwiicPeripheralsInit();
static void InitState2Text(bool state);
static void ButtonTask();
static void CommunicationTask();
static void QwiicWatchDog();
static void MeasureSoilMoistureTask();

static WiFiManager  upstream;
static MqttClient   mqtt(upstream);

static LED *        _leds;
static QwiicButton *_button;
static SoilSens *   _soilSens;
static StopWatch    _timeout(1000);
static StopWatch    _i2ctimeout(200);
static bool         _buttonState = false;

static String name = "Soil Moisture Node";

//MQTT Settings
static String topic = "Irrigation"; //Topic the node is listening to
static int id = 0; //specific id for this node (topic inside topic defined above)
static String reportingString = topic + "/" + id + "/Actual";

void setup() {
  Serial.begin(115200);
  Serial.println("Booting: " + name + ", Node Id: " + id);

  bool init_success = QwiicPeripheralsInit();
  if (!init_success) {
    Serial.println("Initialization failed at some point. Halting...");
    while (1);
  }
  else {
    Serial.println("Peripherals initialized, continuing startup.");
  }

  Serial.print("Resetting WiFi...");
  upstream.reset();
  Serial.println("done");

  Serial.print("Resetting MQTT client...");
  mqtt.reset();
  Serial.println("done");


  Serial.println("Initiating connection to WiFi");
  upstream.connect(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);

  Serial.println("Initiating connection to MQTT broker");
  mqtt.connect(MQTT_CLIENT_ID, MQTT_BROKER_IP, MQTT_BROKER_PORT);

  _timeout.start(1000);
  _i2ctimeout.start(1000);
}

bool QwiicPeripheralsInit() {
    bool init_success = Wire.begin();

  /* We lower the clock here in order to reduce I2C stalls for
     larger QWIIC systems. */
  Wire.setClock(10000);

  Serial.print("Initializing LEDs...");
  {
    _leds = new LED();
    init_success = init_success 
                && _leds->begin();
    init_success = init_success
                && _leds->setLEDBrightness(1);
    InitState2Text(init_success);
  }

  Serial.print("Initializing Button...");
  {
    _button = new QwiicButton();
    init_success = init_success 
                && _button->begin();
    init_success = init_success
                && _button->LEDconfig(1, 0, 0);
    if (init_success) {
      _buttonState = _button->isPressed();
    }
    InitState2Text(init_success);
  }

  Serial.print("Initializing Soil Moisture Sensor...");
  {
    _soilSens = new SoilSens();
    init_success = init_success 
                && _soilSens->begin();
    InitState2Text(init_success);
  }

  return init_success;
}

static void InitState2Text(bool state) {
  Serial.println(state ? "done" : "failed");
}

void loop() {
  CommunicationTask();
  //ButtonTask();
  _soilSens->getValue();
  QwiicWatchDog();
  delay(100);
}

static void CommunicationTask() {
  upstream.run();
  if (upstream.connected()) {
    mqtt.run();
  }
}

static void ButtonTask() {
  if (_buttonState != _button->isPressed()) {
    _buttonState = !_buttonState;
    mqtt.publish("demo/button/state", _buttonState ? "down" : "up");
  }
}

static void MeasureSoilMoistureTask(){
  
}


/**
 * @brief Monitors the QWIIC bus state and resets the bus in case it is hung
 * 
 *        Apparently the QWIIC modules (namely the LED stick) tends to hangup 
 *        the bus. Pobably due to too weak or too strong pull ups when wiring
 *        multiple modules. 
 * 
 *        As a quick fix, the bus status is monitored and the bus re-initialized 
 *        in case of a hang up condition.
 */
static void QwiicWatchDog() {
  _i2ctimeout.restart();
  while (Wire.busy() || (Wire.lastError() != 0)) {
    if (_i2ctimeout.isTimeout()) {
      Serial.println("I2C was busy for too long. Trying to reset.");
      QwiicPeripheralsInit();
      break;
    }
  }
}