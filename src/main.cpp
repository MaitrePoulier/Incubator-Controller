#include <Arduino.h>
////#include "Config.h"
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

//For the DHT-11 humidity sensors
#include "DHT.h"
#define DHTPIN 8   
#define DHTTYPE DHT11   // DHT 11

//For the plot
#define RESOLUTION 0.03125
#define MIN_VALUE 24
#define VERTICAL_HEIGHT  160


//#include "freertos/FreeRTOSConfig.h"
//#include <Wire.h> //pour l'I2C
//#include "BodmerTFT.h"



//Screen pinout define in /pio/libdeps/adafruit.../TFT_eSPI/User_Setup_Select.h
//  then: <User_Setups/TFT_User_Setup_ILI9486.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
DHT dht(DHTPIN, DHTTYPE);

//int Relay = 1;
bool tableau[300][160];
int position;
int x = 0;


void setup() {
  // Some boards work best if we also make a serial connection
  Serial.begin(460800);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(4);
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(6*20);

  dht.begin();

  //initialise the tableau to '0'
  for (int i=0; i<300; i++)
  {
    for (int j = 0; j< 160; j++)
    {
      tableau[i][j] = 0;
    }
  }

  // set LED to be an output pin
  //pinMode(1, OUTPUT);
  
}

void loop() {
  
  //digitalWrite(1 , HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(1000);                // wait for a half second
  //digitalWrite(1 , LOW);    // turn the LED off by making the voltage LOW
  delay(1000);             // wait for a half second


   // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.println(F("%"));

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(F("°C"));

  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.println(F("°C "));


  tft.drawString("Humidity: ", 70,260, 4); //480x320
  tft.drawFloat(h,2,230,260);
  tft.drawString("%", 300, 260, 4); //480x320

  tft.drawString("Temperature: ",70, 280,4);
  tft.drawFloat(t,2,230,280);
  tft.drawString("°C", 300, 280, 4); //480x320


  tft.drawString("Heat index: ",70,300,4); 
  tft.drawFloat(hic,2,230,300);
  tft.drawString("°C", 300, 300, 4); //480x320


  position = (int)((t - MIN_VALUE) / RESOLUTION);
  //puis on inverse pour que le point 0,0 soit en haut à droite
  position = -position + 160;
  Serial.println(position);

  tableau[x][position] = true;


  //plot the tableau
  tft.fillRect(0,0,300,160,TFT_BLACK);
  for (int i=0; i<300; i++) // je suis inversée
  {
    for (int j = 0; j< 160; j++)
    {
      if(tableau[i][j] == true)
      {
        tft.drawPixel(i,j,TFT_RED);
      }
    }
  }
  Serial.println("");
  x++;

}