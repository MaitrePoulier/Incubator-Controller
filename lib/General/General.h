#ifndef _GENERAL_H
    #define _GENERAL_H
    #include <Arduino.h>
    #include <Config.h>

    void SentSerial(Incubator_t *);

    void CheckHumidityAlarm(Incubator_t *, Alarm_t *);
    void CheckTemperatureAlarm(Incubator_t *, Alarm_t *);

#endif