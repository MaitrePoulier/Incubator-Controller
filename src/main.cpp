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


#include <Adafruit_MAX31865.h>





//#include "freertos/FreeRTOSConfig.h"
//#include <Wire.h> //pour l'I2C
//#include "BodmerTFT.h"



//Screen pinout define in /pio/libdeps/adafruit.../TFT_eSPI/User_Setup_Select.h
//  then: <User_Setups/TFT_User_Setup_ILI9486.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

//Initiate humidity and temperature sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();

//int Relay = 1;
int TempTable[TABLE_WIDTH][TABLE_HEIGHT];

unsigned long previousMillis = 0;

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(40,39,3,4);
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      4300.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  1000.0


//Touchscreen
u_int16_t x,y;
bool pressed;

void setup() {
  // Some boards work best if we also make a serial connection
  Serial.begin(460800);

  Draw_screen_background();
  

  Init_Table(TempTable);

  // set LED to be an output pin
  //pinMode(1, OUTPUT);
  
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  thermo.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary
  
  //We draw the buttons
  initButtons();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.setTextDatum(BL_DATUM);
  
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

  uint16_t rtd = thermo.readRTD();

  float ratio = rtd;
  ratio /= 32768;
  Serial.print("Ratio = "); Serial.println(ratio,8);
  Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
  Serial.print("Temperature = "); Serial.println(thermo.temperature(RNOMINAL, RREF));

  Draw_data(37.78, 60.0, h, 25.99, 48.99, 120, 150, t + 0.2, t, thermo.temperature(RNOMINAL, RREF));
  
  //plot the tableau
  Add_Value_Table(TempTable,t+0.1,t,thermo.temperature(RNOMINAL, RREF));
  Draw_Table(TempTable);

  unsigned long currentMillis = millis();
  Display_UpTime(millis());

  Display_Refresh(currentMillis-previousMillis);

  previousMillis = currentMillis;


  Serial.println("");
  pressed = tft.getTouch(&x,&y,MINPRESSURE);
  int State = MainScreenButton(pressed,x,y);
}