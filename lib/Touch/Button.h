#ifndef _BUTTON_H
#define _BUTTON_H

    #include <TFT_eSPI.h>
    #include <TFT_eWidget.h>           // Widget library
    #include "Free_Fonts.h" // Include the header file attached to this sketch
    #include "Config.h"

    void btTup_pressAction(void);
    void btTup_releaseAction(void);
    void btTdown_pressAction(void);
    void btTdown_releaseAction(void);
    void btHup_pressAction(void);
    void btHup_releaseAction(void);
    void btHdown_pressAction(void);
    void btHdown_releaseAction(void);
    void btTiltup_pressAction(void);
    void btTiltup_releaseAction(void);
    void btTiltdown_pressAction(void);
    void btTiltdown_releaseAction(void);
    void btAlarm_pressAction(void);
    void btAlarm_releaseAction(void);


    void initReTareButton(int,int);
    void initButtons(void);

    Button_t MainScreenButton(bool,u_int16_t,u_int16_t);
    int ReTareButton(bool,u_int16_t,u_int16_t);

    #define BUTTON_W 70
    #define BUTTON_H 50

    void displayAlarm(Alarm_t, Alarm_t); 
#endif