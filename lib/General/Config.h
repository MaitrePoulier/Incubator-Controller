#ifndef _CONFIG_H
#define _CONFIG_H

    //For the plot
    #define RESOLUTION 0.03125 //it's the granularity of the RTD decoder
    #define MIN_VALUE 35.2 // 37.7 - 160 * 0.03125  /2 in oC
    #define TABLE_HEIGHT  160
    #define TABLE_WIDTH 390
    #define TILT_DURATION  120000 //2min in ms
    #define TILT_INTERVAL 7200000 //2H in ms

    #define MIN_PID_TIME_WIDTH 25 //in ms
    //For the touchscreen
    #define MINPRESSURE 50
    //#define MAXPRESSURE 10000 //1000 is enough for 10 bits ADC

#endif