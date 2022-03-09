/*
	Interface for Qwiic Soil Moisture Sensor
	========================================
	Based on QwiicLedStick Library By Ciara Jekel @ SparkFun Electronics, June 11th, 2018 
	& 
	Zio Qwiic Soil Moisture Sensor Example 1 - Basic Reading By Harry September 25, 2018
*/

#ifndef Qwiic_SoilSens_h
#define Qwiic_SoilSens_h

#include "Arduino.h"
#include <Wire.h>

#define COMMAND_LED_OFF     	0x00
#define COMMAND_LED_ON      	0x01
#define COMMAND_GET_VALUE   	0x05
#define COMMAND_NOTHING_NEW   	0x99

class SoilSens
{
    // user-accessible "public" interface
  public:
	SoilSens();
    //Initialize parameters such as I2C address and port for communication
	boolean begin(uint8_t address = 0x28, TwoWire &wirePort = Wire);//If user doesn't specificy then 0x28 and Wire will be used
	
	//Test I2C connection on I2C address
	boolean isConnected();

	//Read value from the sensor
	void getValue();

	//Turn LED on Sensor ON or OFF
	void ledOn();
	void ledOff();

	//Change the I2C address from one address to another
	boolean changeAddress(uint8_t newAddress);

  private:
	//This stores the current I2C address of the LED Stick
	uint8_t _soilSensAddress;
	//This stores the requested i2c port
	TwoWire * _i2cPort;
}; 

#endif