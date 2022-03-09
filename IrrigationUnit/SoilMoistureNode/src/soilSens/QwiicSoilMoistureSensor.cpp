/*
	Interface for Qwiic Soil Moisture Sensor                   By Angelo, March 8th, 2022
	=====================================================================================
	Based on QwiicLedStick Library By Ciara Jekel @ SparkFun Electronics, June 11th, 2018 
	& 
	Zio Qwiic Soil Moisture Sensor Example 1 - Basic Reading By Harry, September 25, 2018
*/
 

#include "QwiicSoilMoistureSensor.h"

//Constructor
SoilSens::SoilSens() {
  _soilSensAddress = 0;

  //based on measurements defined limits 
  _upperAdcLimit = 650;
  _lowerAdcLimit = 90;
}

//Start I2C communication
boolean SoilSens::begin(uint8_t address, TwoWire &wirePort) {
  if (address < 0x08 || address > 0x77) return false; //invalid I2C addresses
  _soilSensAddress = address; //store the address in a private global variable
  _i2cPort = &wirePort; //Grab which port the user wants us to use		
  
 return isConnected();
}

//Test I2C connection on I2C address
boolean SoilSens::isConnected() {
  _i2cPort->beginTransmission(_soilSensAddress);
  if (_i2cPort->endTransmission() == 0)
    return true;
  return false;
}


uint16_t SoilSens::getAdcValue() {
  Wire.beginTransmission(_soilSensAddress);
  Wire.write(COMMAND_GET_VALUE); // command for status
  Wire.endTransmission();    // stop transmitting //this looks like it was essential.

  Wire.requestFrom(_soilSensAddress, 2);    // request 1 bytes from slave device qwiicAddress

  uint16_t _ADC_VALUE = 0;

  while (Wire.available()) { // slave may send less than requested
    
    uint8_t ADC_VALUE_L = Wire.read(); 
    uint8_t ADC_VALUE_H = Wire.read();
    _ADC_VALUE=ADC_VALUE_H;
    _ADC_VALUE<<=8;
    _ADC_VALUE|=ADC_VALUE_L;
    Serial.print("ADC_VALUE:  ");
    Serial.println(_ADC_VALUE,DEC);
  }

  uint16_t x=Wire.read(); 
}

uint8_t SoilSens::getPercentageValue(){
  uint16_t adcValue = getAdcValue();

  //make sure adcValue is within limits
  if(adcValue < _lowerAdcLimit)
  {
    adcValue = _lowerAdcLimit;
  }
  else if(adcValue > _upperAdcLimit){
    adcValue = _upperAdcLimit;
  }

  //Calculate percentage value
  adcValue = adcValue - _lowerAdcLimit;
  Serial.print("zeroed ADC Value:  ");
  Serial.println(adcValue);

  uint16_t adcRange = _upperAdcLimit - _lowerAdcLimit;
  Serial.print("ADC Range:  ");
  Serial.println(adcRange);

  double dryPercentageValue = (double)adcValue / adcRange;
  Serial.print("dryPercentage:  ");
  Serial.println(dryPercentageValue);  

  double wetPercentageValue = 1.0-dryPercentageValue;
  Serial.print("wetPercentage:  ");
  Serial.println(wetPercentageValue);

  uint8_t percentageValue = wetPercentageValue * 100;
  Serial.print("Percentage Value:  ");
  Serial.println(percentageValue);

  return percentageValue;
}

void SoilSens::ledOn() {
  Wire.beginTransmission(_soilSensAddress);
  Wire.write(COMMAND_LED_ON);
  Wire.endTransmission();
}

void SoilSens::ledOff() {
  Wire.beginTransmission(_soilSensAddress);
  Wire.write(COMMAND_LED_OFF);
  Wire.endTransmission();
}

