#ifndef _CONFIG_H
    #define _CONFIG_H

    //For the plot
    #define RESOLUTION 0.03125 //it's the granularity of the RTD decoder
    #define MIN_VALUE 21 // 21oC for debugging, 32.8 for normal operation: 37.8 - 160 * 0.03125  /2 in oC
    #define TABLE_HEIGHT  160
    #define TABLE_WIDTH 390
    #define TILT_DURATION  120000 //2min in ms
    #define TILT_INTERVAL 7200000 //2H in ms

    //Alarm threahold in oC
    #define TEMP_ALARM_THRESHOLD 0.4
    #define HUMID_ALARM_THRESHOLD 10

    //Exaustfan Threashold in oC
    #define EXAUST_FAN_THRESHOLD 0.2

    #define MIN_PID_TIME_WIDTH 25 //in ms
    //For the touchscreen
    #define MINPRESSURE 50
    //#define MAXPRESSURE 10000 //1000 is enough for 10 bits ADC

    //Define the 3 CS IO for the 3x PRT on the SPI
    #define CS_1    40
    #define CS_2    5
    #define CS_3    6

    #define   relayHeat     1    //Heat
    #define   relayHumidity 2    //Humidity
    #define   relayFan      41   //Exaust Fan
    #define   relayTiltUp   42   //Tilt up
    #define   relayTiltDown 45   //Tilt Down
    #define   RELAY6 46 
    #define   BUZZER 21 

    // The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
    #define RREF      4300.0

    // The 'nominal' 0-degrees-C resistance of the sensor
    // 100.0 for PT100, 1000.0 for PT1000
    // Calibrated for my 3 specific PRT in Iced water
    //   Start by changer those value to 1000.0, then print the resistance readed at 0oC then put it there
    #define RNOMINAL_TOP  1000.72631836
    #define RNOMINAL_MID  1000.72631836
    #define RNOMINAL_BOT  1000.07019043

    //Uncomment of debugging purpose
    //#define _StopRelay

    /***********************************************************  Buzzer  ***********************************************************/
    #define PWM_Channel     1       // PWM Channel   
    #define Frequency       1000     // PWM frequencyconst
    #define Resolution      8       // PWM resolution ratio
    #define Dutyfactor      200     // PWM Dutyfactor

    //Alarm Type
    typedef enum{
        NONE = 1,
        ALARM = 2,
        ALARM_MUTE = 3
    }Alarm_t;

    typedef enum {
        idle = 0,
        btTempUp = 1,
        btTempDown = 2,
        btHumidUp = 3,
        btHumidDown = 4,    
        btTiltUpStart = 5,
        btTiltStop = 6,
        btTiltDown = 7,
        btAlarmMute = 8
    } Button_t;

    typedef enum {
        TILT_OFF = 0,
        TILT_UP = 1,
        TILT_DOWN = 2
    }Tilt_t;

    typedef struct {
        float setTemp;
        float setHumidity;
        float roomTemp;
        float roomHumidity;
        float chamberHumidity;
        float rtd_1;
        float rtd_2;
        float rtd_3;
        float avrTemp;
        Tilt_t  tilt;
        Tilt_t nextTilt;
        unsigned long tiltTimer;
        unsigned long tiltInterval;
        bool humidifier;
        bool fan;
    } Incubator_t;

#endif