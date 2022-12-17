#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include "max6675.h"
#include "LiquidCrystal_I2C.h"
#include "defs.h"


// Objects
MAX6675 thermocouple(sckPin, csPin, soPin); // create instance object of MAX6675
OneWire oneWire(ONE_WIRE_0);
OneWire oneWire1(ONE_WIRE_1);
DallasTemperature sensor0(&oneWire);
DallasTemperature sensor1(&oneWire1);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// function prototypes
bool tests();
bool lcdTest();
bool thermocoupleTest();
bool DS18B20Test();
void print_lcd(float temp1, float temp2, float temp3);
void controlFlowrate(float tempDifference, float wallTemperature);
void runPump();
void checkTemperatureDifference(float a, float b);


//Global variables
unsigned long lastRead = 0;
float thermocoupleTemp = 0.0;
float DS18B20Temp0 = 0.0;
float DS18B20Temp1 = 0.0;
int pumpSpeed = 0;

void setup(void)
{

  Serial.begin(115200);
  sensor0.begin();
  sensor1.begin();
  tests();
  pinMode(PUMP_PIN,OUTPUT);
  pinMode(HEATER_PIN,OUTPUT);
}
void loop(void)
{
  
  sensor0.requestTemperatures();
  sensor1.requestTemperatures(); // Send the command to get temperature readings
                                 // Serial.println("DONE");
  /********************************************************************/
  float thermocoupleTemp = thermocouple.readCelsius();
  float DS18B20Temp0 = sensor0.getTempCByIndex(0);
  float DS18B20Temp1 = sensor1.getTempCByIndex(0);

  Serial.print("C = ");
  Serial.println(thermocoupleTemp);
  Serial.print("Temp 1 is: ");
  Serial.println(DS18B20Temp0);
  Serial.print("Temp 2 is: ");
  Serial.println(DS18B20Temp1);
  runPump();
  checkTemperatureDifference(DS18B20Temp0,DS18B20Temp1);
  if(millis()-lastRead >= 1000){
    print_lcd(thermocoupleTemp,DS18B20Temp0,DS18B20Temp1);
    lastRead=millis();
  }

 
  
}

bool tests()
{
  while (!lcdTest())
  {
  }
  while (!thermocoupleTest())
  {
  }
  while (!DS18B20Test())
  {
  }
  return true;
}

bool lcdTest()
{
  lcd.init();
  lcd.backlight();
  lcd.print("Maya v0.0.1");
  lcd.setCursor(7,3);
  lcd.print("FYP 18-22");
  delay(1000);
  lcd.clear();
  lcd.print("Starting tests ........");
  delay(1000);
  return true;
}
bool thermocoupleTest()
{
  lcd.clear();
  lcd.print("Running thermocouple test!!!!");
  int count = 0;
  while (count < 10)
  {
    delay(100);
    if (thermocouple.readCelsius() > 0 && thermocouple.readCelsius() < 50)
    {
      count++;
    }
    else
    {
      continue;
    }
  }
  lcd.clear();
  lcd.println("Thermocouple test complete !!!!!!");
  delay(1000);
  return true;
}

bool DS18B20Test()
{
  lcd.clear();
  lcd.println("Running DS18B20 test !!!!!!!!!!");
  delay(1000);
  int count = 0;
  while (count < 10)
  {
    delay(100);
    if (sensor0.getTempCByIndex(0) > 0 && sensor0.getTempCByIndex(0) < 50 && sensor1.getTempCByIndex(0) > 0 && sensor1.getTempCByIndex(0))
    {
      count++;
    }
    else
    {
      continue;
    }
  }
  lcd.clear();
  lcd.println("DS18B20 test complete.....!!!!!");
  delay(1000);
  return true;
}

void print_lcd(float temp1, float temp2, float temp3){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Thermocouple: ");
  lcd.print(temp1);
  lcd.setCursor(0,1);
  lcd.print("Probe 1: ");
  lcd.print(temp2);
  lcd.setCursor(0,2);
  lcd.print("Probe 2: ");
  lcd.print(temp3);

}

/**
 * @brief function to log data
*/
void logData(){

}

/**
 * @brief function to increase flowrate according to temperature
*/
void increaseFlowrate(){
  //implement increased flowrate

}

/**
 * @brief function to decrease flowrate.
*/
void decreaseFlowrate(){

}

/**
 * @brief function to run the pump.
*/
void runPump(){
  // TO IMPLEMENT RUN PUMP
  
}

void checkTemperatureDifference(float a, float b){
  if(a-b>10){
    increaseFlowrate();
  }
  else if (a-b<0){
    decreaseFlowrate();
  }

}
/**
 * @brief function to turn on the heater
*/
void turnOnHeater(){
  digitalWrite(HEATER_PIN,HIGH);

}

/**
 * @brief function to turn off the heater
*/
void turnOffHeater(){
  digitalWrite(HEATER_PIN,LOW);

}

/**
 * @brief Function to turn on the fan
*/
void turnOnFan(){
  digitalWrite(FAN_PIN,HIGH);
}

/**
 * @brief Function to turn the fan off
 * */
void turnOffFan(){
  digitalWrite(FAN_PIN,LOW);
}