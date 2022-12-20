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

LiquidCrystal_I2C lcd(0x27, 20, 4);

// function prototypes
bool tests();
bool lcdTest();
bool thermocoupleTest();
void print_lcd(float temp1, float temp2, float temp3, float flowrate);
void runPump();
void turnOnHeater();
void turnOffHeater();
void turnOnFan();
void turnOffFan();
void checkflowrate();
void increaseFlowrate();

unsigned long lastBlink =0;

float lastReadThermocouple = 0.0;
int triac_delay = 2500;
int constrained_triac_delay = constrain(triac_delay, 0, 5000);
short int pump_flag = 0;

float thermocoupleTemp = 0.0;

short int temp_change_flag = 0;
float flowrate =0.0;

int initial_time;
int current_time;
int prev_time;
int var1 = 0;
int init_time_flag = 0;



int fx;
int fy;
float ftime;
float f_frequency;
float ftotal;
float f_flm;
float f_fls;
float f_flh;


void setup(void)
{

  Serial.begin(115200);
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(FLOWRATE_PIN,INPUT);
  
  while(!tests())
  {
    if(millis()-lastBlink >= 500)
    {
      digitalWrite(WHITE_LED,!digitalRead(WHITE_LED));
      lastBlink =millis();
    }
  };

  delay(setup_time);
  attachInterrupt(ZERO_CROSS_PIN, runPump, CHANGE);
  prev_time = millis();
}




void loop(void)
{
  thermocoupleTemp = thermocouple.readCelsius();
  checkflowrate();
  switch(var1)
  {
    case 0:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("IDLE STATE");
      lcd.setCursor(1,0);
      lcd.print("Temp: ");
      lcd.print(thermocoupleTemp);
      lcd.print("deg");
      lcd.setCursor(2,0);
      lcd.print("Q: ");
      lcd.print(f_flm);
      lcd.print("l/m");
      lcd.setCursor(3,0);
      lcd.print("TEST1: 1 :: TEST2: 2");
      pump_flag = 0;
      turnOffFan();
      // if(keypad_pressed)
      // {
      //   var1 = 2;
      // }
      break;
    case 1:
      turnOnFan();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("CONSTANT STATE");
      lcd.setCursor(1,0);
      lcd.print("Temp: ");
      lcd.print(thermocoupleTemp);
      lcd.print("deg");
      lcd.setCursor(2,0);
      lcd.print("Q: ");
      lcd.print(f_flm);
      lcd.print("l/m");
      lcd.setCursor(3,0);
      lcd.print("CANCEL: C");
      triac_delay = 2500;
      constrained_triac_delay = constrain(triac_delay, 0, 5000);
      break;

    case 2:
      if(init_time_flag == 0)
      {
        initial_time = millis();
        init_time_flag = 1;
      }
      turnOnFan();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("DYNAMIC STATE");
      lcd.setCursor(1,0);
      lcd.print("Temp: ");
      lcd.print(thermocoupleTemp);
      lcd.print("deg");
      lcd.setCursor(2,0);
      lcd.print("Q: ");
      lcd.print(f_flm);
      lcd.print("l/m");
      lcd.setCursor(3,0);
      lcd.print("CANCEL: C");
      
      current_time = millis();
      if(current_time - initial_time >= 5000)
      {
        lastReadThermocouple = thermocoupleTemp;
      }
      if(thermocoupleTemp - lastReadThermocouple > 5)
      {
        increaseFlowrate();
      }
      break;
    default:
      var1 = 0;
      break;

  }

  // if (temp_change_flag == 0)
  // {
  //   thermocoupleTemp += 1;
  //   if(thermocoupleTemp >=25)
  //   {
  //     temp_change_flag = 1;
  //   }
  // }
  // else
  // {
  //   thermocoupleTemp -= 1;
  //   if(thermocoupleTemp <=0)
  //   {
  //     temp_change_flag = 0;
  //   }
  // }
  
  Serial.print("Thermocouple temp = ");
  Serial.println(thermocoupleTemp);
  Serial.print("Last Thermocouple temp = ");
  Serial.println(lastReadThermocouple);
  Serial.print("Triac delay is: ");
  Serial.println(constrained_triac_delay);
  current_time = millis();

}

bool tests()
{
  while (!lcdTest())
  {
  }
  while (!thermocoupleTest())
  {
  }

  return true;
}

bool lcdTest()
{
  lcd.init();
  lcd.backlight();
  lcd.print("Maya v0.0.1");
  lcd.setCursor(7, 3);
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



void print_lcd(float temp1, float temp2, float temp3, float flowrate)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Thermocouple: ");
  lcd.print(temp1);
  lcd.setCursor(0, 1);
  lcd.print("Probe 1: ");
  lcd.print(temp2);
  lcd.setCursor(0, 2);
  lcd.print("Probe 2: ");
  lcd.print(temp3);
  lcd.setCursor(0,3);
  lcd.print("Flowrate:");
  lcd.print(flowrate);
  lcd.print("L/m");
}

/**
 * @brief function to increase flowrate according to temperature
 */
void increaseFlowrate()
{
  // implement increased flowrate
triac_delay -= 500;
constrained_triac_delay = constrain(triac_delay, 0, 5000);
 
}

/**
 * @brief function to decrease flowrate.
 */
void decreaseFlowrate()
{
  triac_delay += 500;
  constrained_triac_delay = constrain(triac_delay, 0, 5000);
  
}

/**
 * @brief function to run the pump.
 */
void runPump()
{
  // TO IMPLEMENT RUN PUMP
  if(pump_flag == 0)
  {
    constrained_triac_delay = constrained_triac_delay;
  }
  else
  {
    delayMicroseconds(constrained_triac_delay);
    digitalWrite(PUMP_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(PUMP_PIN, LOW);
  }

}
void checkflowrate()
{
   fx = pulseIn(FLOWRATE_PIN, HIGH);
   fy = pulseIn(FLOWRATE_PIN, LOW);
   ftime = fx + fy;
   f_frequency = 1000000/ftime;
   f_flm = f_frequency/7.5;
   f_fls = f_flm/60.0;
   f_flh = f_flm*60;


}


/**
 * @brief function to turn on the heater
 */
void turnOnHeater()
{
  digitalWrite(HEATER_PIN, HIGH);
}

/**
 * @brief function to turn off the heater
 */
void turnOffHeater()
{
  digitalWrite(HEATER_PIN, LOW);
}

/**
 * @brief Function to turn on the fan
 */
void turnOnFan()
{
  digitalWrite(FAN_PIN, LOW);
}

/**
 * @brief Function to turn the fan off
 * */
void turnOffFan()
{
  digitalWrite(FAN_PIN, HIGH);
}

// void full_speed()
// {
//   triac_delay = 0;
//   pump_flag = 0;
//   for(int full_speed = 0; full_speed <= 6; full_speed++)
//   {
//     delay(1000);
//     digitalWrite(PUMP_PIN, HIGH);
//     Serial.print("Full speed temp. data is : ");
//     Serial.println("last value");
//   }
//   pump_flag = 1;
// }
// float checkFlowrate()
// {
//   int X;
//   int Y;
//   float TIME = 0;
//   float FREQUENCY = 0;
//   float WATER = 0;
//   X = pulseIn(FLOWRATE_PIN, HIGH);
//   Y = pulseIn(FLOWRATE_PIN, LOW);
//   TIME = X + Y;
//   FREQUENCY = 1000000/TIME;
//   WATER = FREQUENCY/7.5;

//   if(FREQUENCY >= 0)
//   {
//   if(isinf(FREQUENCY)) return NULL;

//   else{return WATER;}

//   }
// }
