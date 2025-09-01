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

// Task handles
TaskHandle_t TaskPID;
// Queue handle for inter-core communication
QueueHandle_t avrTempQueue;

//For the PID
#include <PID_v2.h>
// Specify the links and initial tuning parameters
double Kp = 2, Ki = .1, Kd = 0;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

int   WindowSize  = 1000;
unsigned long windowStartTime;
double output;


#include <Adafruit_MAX31865.h>

float setTemp = 37.8;
float setHumidity = 50;
int   Tilt = 0; //no tilt
int   NextTilt = 1; //go up next time
unsigned long   TiltTimer    =  120000; // The duration of the tile in ms: 2min
unsigned long   TiltInterval = 7200000; //Interval before 2 tilt in ms: 2h
#define   RELAY1 1    //Heat
#define   RELAY2 2    //Humidity
#define   RELAY3 41   //Exaust Fan
#define   RELAY4 42   //Tilt up
#define   RELAY5 45   //Tilt Down
#define   RELAY6 46 
#define   BUZZER 21 


bool Humidifier = false; //set if the humidifier is on or off
bool Fan = false; //set if the Output Fan is on or off

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
TwoWire I2C_1 = TwoWire(0);
TwoWire I2C_2 = TwoWire(1);
Adafruit_SHT31 sht31_room = Adafruit_SHT31();
Adafruit_SHT31 sht31_chamber = Adafruit_SHT31();

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo1 = Adafruit_MAX31865(CS_1,39,3,4); //left  (top of the chamber - Red)
Adafruit_MAX31865 thermo2 = Adafruit_MAX31865(CS_2,39,3,4); //middle (Middle of the chamber - Green)
Adafruit_MAX31865 thermo3 = Adafruit_MAX31865(CS_3,39,3,4);  //right (Bot of the chamber - Blue)
float rtd_1;
float rtd_2;
float rtd_3;
float avrTemp = 30;



//Touchscreen
u_int16_t x,y;
bool pressed;
int count;

//Alarm Type
enum Alarm{
  NONE,
  ALARM,
  ALARM_MUTE
};

enum Alarm HumidityAlarm = ALARM;
enum Alarm TemperatureAlarm = ALARM;

// Function that implement the PID
// We put it on core 0 to be able to control precisely the time it take to execute
void PIDCtrl(void *pvParameters) {
 
  while (true) {
    // Check if a new potentiometer value is available in the queue
    if (xQueueReceive(avrTempQueue, &avrTemp, 0) == pdPASS) {
      // Received a new potentiometer value
    }
 
    //temperature asservissement
    output = myPID.Run(avrTemp);

    /************************************************
     * turn the output pin on/off based on pid output
     ************************************************/
    while (millis() - windowStartTime > WindowSize) {
      // time to shift the Relay Window
      windowStartTime += WindowSize;
    }
    //Serial.println(output);
    //Serial.println(millis());
    //Serial.println(windowStartTime);
    //Serial.print("setpoint: ");
    //Serial.println(myPID.GetSetpoint());
    #ifndef _StopRelay
      if (output > millis() - windowStartTime && output > MIN_PID_TIME_WIDTH + 0.01)
      {
        digitalWrite(RELAY1,HIGH);

        //tft.setTextColor(TFT_RED, TFT_BLACK);
        //tft.drawString("Heat On ", 390, 190,2);
        //tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }
      else
      {
        digitalWrite(RELAY1,LOW);
        //tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        //tft.drawString("Heat Off", 390, 190,2);
        //tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }
    #endif

  //End temperature asservissement
  //Serial.print("PID is on core: ");
  //Serial.println(xPortGetCoreID());
  vTaskDelay(10);
  }
}


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
  pinMode(BUZZER, OUTPUT);

  /////////////////////////
  // Initialise both SHT31
  //https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/ 
  I2C_1.begin(8,9,100000);
  I2C_2.begin(35,36,100000);
  sht31_room = Adafruit_SHT31(&I2C_1);
  sht31_chamber = Adafruit_SHT31(&I2C_2);

  Serial.println("SHT31 room test");
  if (! sht31_room.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31 room");
    while (1) delay(1);
  }

  Serial.println("SHT31 chamber test");
  if (! sht31_chamber.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31 chamber");
    while (1) delay(1);
  }
  //////////////////


  thermo1.begin(MAX31865_3WIRE); 
  thermo2.begin(MAX31865_3WIRE); 
  thermo3.begin(MAX31865_3WIRE); 
  
  //We draw the buttons
  initButtons();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.setTextDatum(BL_DATUM);

  //PID
  windowStartTime = millis();
  // tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(MIN_PID_TIME_WIDTH, WindowSize);
  // turn the PID on
  myPID.Start(thermo1.temperature(RNOMINAL_TOP, RREF),  // input
              0,                      // current output
              setTemp);                   // setpoint

  // Create a queue capable of holding one float value
  avrTempQueue = xQueueCreate(1, sizeof(float));
  
  // Create the PID controller task on core 0
  xTaskCreatePinnedToCore(
    PIDCtrl,       // Task function
    "TaskPID",            // Name of the task
    10000,               // Stack size (bytes)  at 1000 it make a stack overflow
    NULL,               // Parameter to pass to the task
    2,                  // Task priority
    &TaskPID,             // Task handle
    0);                 // Core to run the task (Core 0)

  TiltInterval = TILT_INTERVAL + millis(); //2h waiting time at start

  //For the ESP32 PWM channel
  ledcSetup(PWM_Channel, Frequency, Resolution);            // Set a LEDC channel
  ledcAttachPin(BUZZER, PWM_Channel);              // Connect the channel to the corresponding pin
  
}


void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float room_h = sht31_room.readHumidity();
  float chamber_h = sht31_chamber.readHumidity();
  // Read temperature as Celsius (the default)
  float room_t = sht31_room.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(room_h) || isnan(room_t)) {
    Serial.println(F("Failed to read from DHT room sensor!"));
    return;
  }
  if (isnan(chamber_h)) {
    Serial.println(F("Failed to read from DHT chamber sensor!"));
    return;
  }


  
  Serial.print(F("Room Humidity: "));
  Serial.print(room_h);
  Serial.println(F("%"));

  Serial.print(F("Chamber  Humidity: "));
  Serial.print(chamber_h);
  Serial.println(F("%"));

  Serial.print("Room Temperature: ");
  Serial.print(room_t);
  Serial.println(F("oC"));
  

  rtd_1 = thermo1.temperature(RNOMINAL_TOP, RREF); //the function is blocking. It cost 75ms
  rtd_2 = thermo2.temperature(RNOMINAL_MID, RREF);
  rtd_3 = thermo3.temperature(RNOMINAL_LOW, RREF);
  avrTemp = (rtd_1 + rtd_2 + rtd_3)/3;


  // Send the average temperature of the 3 RTD to the PID task
  // The xQueueSend wait after the PID to remove the value from the Queue
  xQueueSend(avrTempQueue, &avrTemp, portMAX_DELAY);
  Serial.print("avrg. temps = "); Serial.println(avrTemp);

  Draw_data(setTemp, setHumidity, room_t, room_h, int((TiltInterval - millis())/1000/60), 150, chamber_h, rtd_1, rtd_2, rtd_3);
  

  // for now we update the plot only every 10 cycles or 10 x 140ms = 1.4s
  // later on we will want to update it every minutes
  count ++;
  if (count > 9) 
  {
    count = 0;
    //plot the tableau
    Add_Value_Table(TempTable,rtd_1, rtd_2, rtd_3);
    Draw_Table(TempTable);
    //We only save 20ms with the sprite method over writing directly on the screen... 
    //But it's better than nothing!!
    TempTable_Spr.pushSprite(1,1); //We offset to clear the whiteborder
  }
  

  //Humidity
  if (chamber_h <= setHumidity -5 )
  {
    Humidifier = true;
    digitalWrite(RELAY2,HIGH);
  }
  if (chamber_h > setHumidity)
  {
    Humidifier = false;
    digitalWrite(RELAY2,LOW);
  }

  //OverTemp
  if (avrTemp >= setTemp + 0.2)
  {
    Fan = true;
    digitalWrite(RELAY3,HIGH);
  }
  else
  {
    Fan = false;
    digitalWrite(RELAY3,LOW);
  }
  
  //Tilt
  if (TiltInterval < millis())
  {
    TiltInterval = TILT_INTERVAL + millis(); //we add a new 2h
    Tilt = NextTilt; //we start the tilting process
    TiltTimer = millis() + TILT_DURATION; //2min tilting time
    if (Tilt == 1) NextTilt = 2; //We planne the next Tilt
    if (Tilt == 2) NextTilt = 1;
  }
  if (TiltTimer < millis()) Tilt = 0;

  switch (Tilt)
  {
  case 1: //Tilt up
    digitalWrite(RELAY5,LOW);   //down relay
    digitalWrite(RELAY4,HIGH);  //up relay
    break;

  case 2: //Tilt Down 
    digitalWrite(RELAY4,LOW);   //up relay
    digitalWrite(RELAY5,HIGH);  //down relay
    break;

  case 0: //no Tilt
    digitalWrite(RELAY4,LOW);   //up relay
    digitalWrite(RELAY5,LOW);   //down relay
    break;
  }
  
  //Ventilation

  
  //We check if somebody have touch the button 
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
      setHumidity = setHumidity + 1;
      break;
    case 4:
      setHumidity = setHumidity - 1;
      break;
    case 5:
      Tilt = 1; //up
      TiltInterval = TILT_INTERVAL + millis(); //we add a new 2h
      break;   
    case 6:
      Tilt = 0; //Stop
      break;   
    case 7:
      Tilt = 2; //down
      TiltInterval = TILT_INTERVAL + millis(); //we add a new 2h
      break;  
    default:
      break;    
  }

  Display_Heater(output); //Display heater on/off
  Display_Humidifier(Humidifier);
  Display_Fan(Fan);
  Display_Tilt(Tilt);

  currentMillis = millis();
  Display_UpTime(currentMillis);
  Display_Refresh(currentMillis-previousMillis);
  previousMillis = currentMillis;
  Serial.println("");

  if(HumidityAlarm == ALARM)
  {
      ledcWrite(PWM_Channel, Dutyfactor);
      delay(100);
      ledcWrite(PWM_Channel, 0);
  };
      delay(100);
  if(TemperatureAlarm == ALARM)
  {
      ledcWrite(PWM_Channel, Dutyfactor);
      delay(100);
      ledcWrite(PWM_Channel, 0);
      delay(100);
      ledcWrite(PWM_Channel, Dutyfactor);
      delay(100);
      ledcWrite(PWM_Channel, 0);
  };

}