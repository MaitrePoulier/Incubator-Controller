#include <Arduino.h>
#include <Config.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ScreenStuff.h>

#include <Button.h> //Pour l'écran de boutons

//For the DHT-11 humidity sensors
// Default I2C pins with the esp32-s3-devkitc-1
// GPIO 8 (SDA)  SHT30 White Wire
// GPIO 9 (SCL)  SHT30 Yellow Wire 
#include <Wire.h>
#include "Adafruit_SHT31.h"

//For the PID
#include <PID_v2.h>
// Specify the links and initial tuning parameters
double Kp = 1, Ki = 0.1, Kd = 0.25;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

int   WindowSize  = 5000;
unsigned long windowStartTime;


#include <Adafruit_MAX31865.h>

float setTemp = 34;
float setHumidity = 50.1;
int   Tilt = 0; //no tilt
bool  Relay_heat = false; // relay #1 for heater
#define   RELAY1 1 
#define   RELAY2 2 
#define   RELAY3 41 
#define   RELAY4 42 
#define   RELAY5 45
#define   RELAY6 46 


unsigned long currentMillis;
unsigned long previousMillis = 0;

//#include "freertos/FreeRTOSConfig.h"
//#include <Wire.h> //pour l'I2C
//#include "BodmerTFT.h"



//Screen pinout define in /pio/libdeps/adafruit.../TFT_eSPI/User_Setup_Select.h
//  then: <User_Setups/TFT_User_Setup_ILI9486.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
uint16_t TempTable[TABLE_WIDTH][TABLE_HEIGHT];
TFT_eSprite TempTable_Spr = TFT_eSprite(&tft); // Declare Sprite object "spr" with pointer to "tft" object

//Initiate humidity and temperature sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo1 = Adafruit_MAX31865(40,39,3,4);
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      4300.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  1000.0
float rtd_1;


//Touchscreen
u_int16_t x,y;
bool pressed;

int count;

void setup() {
  // Some boards work best if we also make a serial connection
  Serial.begin(460800);

  Draw_screen_background();
  

  Init_Table(TempTable);
  // Create a sprite of defined size
  TempTable_Spr.createSprite(TABLE_WIDTH-3,TABLE_HEIGHT-3);
  // Fill the whole sprite with black (Sprite is in memory so not visible yet)
  TempTable_Spr.fillSprite(TFT_BLACK);

  // set all relay IO to output
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(RELAY5, OUTPUT);
  pinMode(RELAY6, OUTPUT);
  
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  thermo1.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary
  
  //We draw the buttons
  initButtons();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.setTextDatum(BL_DATUM);

  

  //PID
  windowStartTime = millis();

  // tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(180, WindowSize); //loop() take at least 180ms for now

  // turn the PID on
  myPID.Start(thermo1.temperature(RNOMINAL, RREF),  // input
              0,                      // current output
              setTemp);                   // setpoint
  
}

void loop() {
  
  //digitalWrite(1 , HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(1000);                // wait for a half second
  //digitalWrite(1 , LOW);    // turn the LED off by making the voltage LOW


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = sht31.readHumidity();
  // Read temperature as Celsius (the default)
  float t = sht31.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.println(F("%"));

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(F("°C"));
  
  rtd_1 = thermo1.temperature(RNOMINAL, RREF); //the function is blocking. It cost 75ms
  Serial.print("Temperature = "); Serial.println(rtd_1);

  Draw_data(setTemp, setHumidity, t, 48.99, 120, 150, h, rtd_1+0.1, rtd_1, rtd_1-0.1);
  
  count ++;
  if (count > 9)
  {
    count = 0;
  //plot the tableau
  Add_Value_Table(TempTable,rtd_1+0.1, rtd_1, rtd_1-0.1);
  Draw_Table(TempTable);
  //We only save 20ms with the sprite method over writing directly on the screen... 
  //But it's better than nothing!!
  TempTable_Spr.pushSprite(1,1); //We offset to clear the whiteborder
  }
  
  //temperature asservissement
  double output = myPID.Run(rtd_1);

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
  while (millis() - windowStartTime > WindowSize) {
    // time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  Serial.println(output);
  Serial.println(millis());
  Serial.println(windowStartTime);
  if (output > millis() - windowStartTime && output > 180.5)
  {
    digitalWrite(RELAY1,HIGH);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Heat On ", 390, 190,2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  else
  {
    digitalWrite(RELAY1,LOW);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Heat Off", 390, 190,2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }

  //End temperature asservissement



  Serial.println("");
  
  pressed = tft.getTouch(&x,&y,MINPRESSURE);
  switch (MainScreenButton(pressed,x,y)) {
    case 1:
      setTemp = setTemp + 0.1;
      myPID.Setpoint(setTemp);
      break;
    case 2:
      setTemp = setTemp - 0.1;
      myPID.Setpoint(setTemp);
      break;
    case 3:
      setHumidity = setHumidity + 0.1;
      break;
    case 4:
      setHumidity = setHumidity - 0.1;
      break;
    case 5:
      Tilt = 1; //up
      break;   
    case 6:
      Tilt = 0; //Stop
      break;   
    case 7:
      Tilt = 2; //down
      break;  
    default:
      break;    
  }


  currentMillis = millis();
  Display_UpTime(millis());
  Display_Refresh(currentMillis-previousMillis);
  previousMillis = currentMillis;
}