#include <Arduino.h>
#include <Config.h>
#include <General.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ScreenStuff.h>
#include <Button.h> //Pour l'écran de boutons
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <ESP32Time.h>

// Task handles
TaskHandle_t TaskPID;
TaskHandle_t TaskBuzzer;
// Queue handle for inter-core communication
QueueHandle_t avrTempQueue;


// C'est laid, mais je ne sais pas comment l'envoyer dans l'autre core sans ça
// Ça sers au PID
float avrTemp = 30;
float setTemp = 37.5;

//For the PID
#include <PID_v2.h>
// Specify the links and initial tuning parameters
double Kp = 2, Ki = .1, Kd = 0;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

int   WindowSize  = 1000;
unsigned long windowStartTime;
double output;


#include <Adafruit_MAX31865.h>

//For the refresh rate
unsigned long currentMillis;
unsigned long previousMillis = 0;

//Initiate the RTC for the uptime display
ESP32Time rtc;

//Screen pinout define in /pio/libdeps/adafruit.../TFT_eSPI/User_Setup_Select.h
//  then: <User_Setups/TFT_User_Setup_ILI9486.h>
#include <../TFT_eSPI_Setup/User_Setup_Select.h>
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


//Variables for the touchscreen
u_int16_t x,y;
bool pressed;
int count;

//2 global variable to handle alarms
Alarm_t  HumidityAlarm = NONE;
Alarm_t TemperatureAlarm = NONE;

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
        digitalWrite(relayHeat,HIGH);

        //tft.setTextColor(TFT_RED, TFT_BLACK);
        //tft.drawString("Heat On ", 390, 190,2);
        //tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }
      else
      {
        digitalWrite(relayHeat,LOW);
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
void Buzzer(void *pvParameters) {
  while (true) {

    if(HumidityAlarm == ALARM)
    {
        ledcWrite(PWM_Channel, Dutyfactor);
        vTaskDelay(100);
        ledcWrite(PWM_Channel, 0);
    };
    
    vTaskDelay(500);

    if(TemperatureAlarm == ALARM)
    {
        ledcWrite(PWM_Channel, Dutyfactor);
        vTaskDelay(100);
        ledcWrite(PWM_Channel, 0);
        vTaskDelay(100);
        ledcWrite(PWM_Channel, Dutyfactor);
        vTaskDelay(100);
        ledcWrite(PWM_Channel, 0);
    };

    vTaskDelay(500);
  };
};

void Read3PRT(Incubator_t *);

void setup() {
  // Some boards work best if we also make a serial connection
  Serial.begin(460800);

  // 1st Jan 2021 00:00:00
  rtc.setTime(1609459200);  

  Draw_screen_background();
  
  Init_Table(TempTable);
  // Create a sprite of defined size
  TempTable_Spr.createSprite(TABLE_WIDTH-3,TABLE_HEIGHT-3);
  // Fill the whole sprite with black (Sprite is in memory so not visible yet)
  TempTable_Spr.fillSprite(TFT_BLACK);

  // set all relay IO to output
  pinMode(relayHeat, OUTPUT);
  pinMode(relayHumidity, OUTPUT);
  pinMode(relayFan, OUTPUT);
  pinMode(relayTiltUp, OUTPUT);
  pinMode(relayTiltDown, OUTPUT);
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

  //PID
  windowStartTime = millis();
  // tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(MIN_PID_TIME_WIDTH, WindowSize);
  // turn the PID on
  myPID.Start(thermo1.temperature(RNOMINAL_TOP, RREF),  // input
              0,                      // current output
              setTemp);     // setpoint

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
    0);                 // Core to run the task (Core 0
    
  // Create the Buzzer task on core 0
  xTaskCreatePinnedToCore(
    Buzzer,       // Task function
    "TaskBuzzer",            // Name of the task
    10000,               // Stack size (bytes)  at 1000 it make a stack overflow
    NULL,               // Parameter to pass to the task
    2,                  // Task priority
    &TaskBuzzer,             // Task handle
    0);                 // Core to run the task (Core 0)


  //For the ESP32 PWM channel
  ledcSetup(PWM_Channel, Frequency, Resolution);   // Set a LEDC channel
  ledcAttachPin(BUZZER, PWM_Channel);              // Connect the channel to the corresponding pin
  
  Serial.println("Initialisation completed!");
}


void loop() {

  //Define a structure to hold incubator parameters
  //Static so it's only created once
  //I try to limit the memory fragmentation by putting everything global.
  static Incubator_t incubator = {
      .setTemp = 37.7,
      .setHumidity = 50.0,
      .roomTemp = 22.0,
      .roomHumidity = 40.0,
      .chamberHumidity = 45.0,
      .rtd_1 = 37.0,
      .rtd_2 = 37.0,
      .rtd_3 = 37.0,
      .avrTemp = 37.0,
      .tilt = TILT_OFF,
      .nextTilt = TILT_UP,
      .tiltTimer = 120000,      // The duration of the tile in ms: 2min
      .tiltInterval = TILT_INTERVAL + millis(),  //Interval before 2 tilt in ms: 2h
      .humidifier = false,  //set if the humidifier is on or off
      .fan = false          //set if the Output Fan is on or off
  };


  // Reading temperature or humidity takes about 50 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  incubator.roomHumidity = sht31_room.readHumidity();
  incubator.chamberHumidity = sht31_chamber.readHumidity();
  // Read temperature as Celsius (the default)
  incubator.roomTemp = sht31_room.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(incubator.roomHumidity) || isnan(incubator.roomTemp)) {
    Serial.println(F("Failed to read from DHT room sensor!"));
    return;
  }
  if (isnan(incubator.chamberHumidity)) {
    Serial.println(F("Failed to read from DHT chamber sensor!"));
    return;
  }

  // We read the 3 RTD
  // change the value of rtd_1, rtd_2, rtd_3 and avrTemp
  Read3PRT(&incubator);

  // Send the average temperature of the 3 RTD to the PID task
  // The xQueueSend wait after the PID to remove the value from the Queue
  xQueueSend(avrTempQueue, &incubator.avrTemp, portMAX_DELAY);

  Draw_data(&incubator);
  
  /******************** Update the temperature plot **************** */
  //
  // for now we update the plot only every 30 cycles or 30 x 30ms = 900ms
  // later on we will want to update it every minutes
  count ++;
  if (count > 30) 
  {
    count = 0;
    //plot the tableau
    Add_Value_Table(TempTable,incubator.rtd_1, incubator.rtd_2, incubator.rtd_3);
    Draw_Table(TempTable);
    //We only save 20ms with the sprite method over writing directly on the screen... 
    //But it's better than nothing!!
    TempTable_Spr.pushSprite(1,1); //We offset to clear the whiteborder
  }
  
  /******************** Humidity **************** */
  // No fancy PID here
  //
  if (incubator.chamberHumidity <= incubator.setHumidity -5 )
  {
    incubator.humidifier = true;
    digitalWrite(relayHumidity,HIGH);
  }
  //we create a small hysteresis to avoid relay oscillation
  else if (incubator.chamberHumidity > incubator.setHumidity )
  {
    incubator.humidifier = false;
    digitalWrite(relayHumidity,LOW);
  }

  CheckHumidityAlarm(&incubator, &HumidityAlarm);

  /******************** Temperature and exaust fan **************** */
  // Temperature is controlled in a the PID task, not here

  if (incubator.avrTemp >= incubator.setTemp + EXAUST_FAN_THRESHOLD)
  {
    incubator.fan = true;
    digitalWrite(relayFan,HIGH);
  }
  else
  {
    incubator.fan = false;
    digitalWrite(relayFan,LOW);
  }

  CheckTemperatureAlarm(&incubator, &TemperatureAlarm);



  /******************** Tilt **************** */
  if (incubator.tiltInterval < millis())
  {
    incubator.tiltInterval = TILT_INTERVAL + millis(); //we add a new 2h
    incubator.tilt = incubator.nextTilt; //we start the tilting process
    incubator.tiltTimer = millis() + TILT_DURATION; //2min tilting time
    if (incubator.tilt == TILT_UP) incubator.nextTilt = TILT_DOWN; //We planne the next Tilt
    if (incubator.tilt == TILT_DOWN) incubator.nextTilt = TILT_UP;
  }
  if (incubator.tiltTimer < millis()) incubator.tilt = TILT_OFF;

  switch (incubator.tilt)
  {
  case TILT_UP: 
    digitalWrite(relayTiltDown,LOW);   //down relay
    digitalWrite(relayTiltUp,HIGH);  //up relay
    break;

  case TILT_DOWN: 
    digitalWrite(relayTiltUp,LOW);   //up relay
    digitalWrite(relayTiltDown,HIGH);  //down relay
    break;

  case TILT_OFF: 
    digitalWrite(relayTiltUp,LOW);   //up relay
    digitalWrite(relayTiltDown,LOW);   //down relay
    break;
  }
  
  
  //We check if somebody have touch the button 
  pressed = tft.getTouch(&x,&y,MINPRESSURE);
  switch (MainScreenButton(pressed,x,y)) {
    case idle:
      break;
    case btTempUp:
      incubator.setTemp += 0.1;
      myPID.Setpoint(setTemp);
      break;
    case btTempDown:
      incubator.setTemp -= 0.1;
      myPID.Setpoint(incubator.setTemp);
      break;
    case btHumidUp:
      incubator.setHumidity += 1;
      break;
    case btHumidDown:
      incubator.setHumidity -= 1;
      break;
    case btTiltUpStart:
      incubator.tilt = TILT_UP; //up
      incubator.tiltInterval = TILT_INTERVAL + millis(); //we add a new 2h
      break;   
    case btTiltStop:
      incubator.tilt = TILT_OFF; //Stop
      break;   
    case btTiltDown:
      incubator.tilt = TILT_DOWN; //down
      incubator.tiltInterval = TILT_INTERVAL + millis(); //we add a new 2h
      break;  
    case btAlarmMute: //Mute alarm if on
      if(HumidityAlarm == ALARM) HumidityAlarm = ALARM_MUTE;
      if(TemperatureAlarm == ALARM) TemperatureAlarm = ALARM_MUTE;
      break;  

    default:
      break;    
  }

  //***************   Update display   *************** */

  Display_Heater(output); //Display heater on/off
  Display_Humidifier(incubator.humidifier);
  Display_Fan(incubator.fan);
  Display_Tilt(incubator.tilt);

  //We remove the offset to have the uptime since the 1st Jan 2021
  Display_UpTime(rtc.getEpoch()-1609459200); 


  currentMillis = millis();
  Display_Refresh(currentMillis-previousMillis);
  previousMillis = currentMillis;
  
  // We send the value to the serial for logging purpose
  //SentSerial(&incubator);
  //Serial.println("");

  displayAlarm(HumidityAlarm, TemperatureAlarm);
}

void Read3PRT(Incubator_t *incubator)
{
  // read data x3 and convert to temperature
  incubator->rtd_1 = thermo1.calculateTemperature(thermo1.readData(), RNOMINAL_TOP, RREF);
  incubator->rtd_2 = thermo2.calculateTemperature(thermo2.readData(), RNOMINAL_MID, RREF);
  incubator->rtd_3 = thermo3.calculateTemperature(thermo3.readData(), RNOMINAL_BOT, RREF);

  incubator->avrTemp = (incubator->rtd_1 + incubator->rtd_2 + incubator->rtd_3) / 3;
  
  //Très laid, mais je ne sais pas comment faire autrement pour l'instant
  //Sers au PID
  avrTemp = incubator->avrTemp;
  setTemp = incubator->setTemp;

  // We start the reading of the 3 RTD
  // It take at least 50ms to be ready.
  // Clear fault x3
  thermo1.ClearFault();
  thermo2.ClearFault();
  thermo3.ClearFault();

  // ask for data x3
  thermo1.askData();
  thermo2.askData();
  thermo3.askData();
}
