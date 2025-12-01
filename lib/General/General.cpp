#include <General.h>
#include <Config.h>

void SentSerial(Incubator_t *incubator){

  Serial.print(F("Room Humidity: "));
  Serial.print(incubator->roomHumidity);
  Serial.println(F("%"));

  Serial.print(F("Chamber  Humidity: "));
  Serial.print(incubator->chamberHumidity);
  Serial.println(F("%"));

  Serial.print("Room Temperature: ");
  Serial.print(incubator->roomTemp);
  Serial.println(F("oC"));

  Serial.print("avrg. temps = "); 
  Serial.println(incubator->avrTemp);
}

void CheckTemperatureAlarm(Incubator_t *incubator, Alarm_t *TemperatureAlarm)
{
  //Si l'alarm est mute et que l'humidité est redevenue normale, on l'enlève
  if(*TemperatureAlarm == ALARM_MUTE && 
    ((incubator->avrTemp <= incubator->setTemp + TEMP_ALARM_THRESHOLD) && 
    (incubator->avrTemp >= incubator->setTemp - TEMP_ALARM_THRESHOLD)))
  {
    *TemperatureAlarm = NONE;
    return;
  }
  //Si on est muté, on quitte
  if(*TemperatureAlarm == ALARM_MUTE) return;

  //Si l'humidité est trop haute ou basse on met l'alarm.
  if((incubator->avrTemp > incubator->setTemp + TEMP_ALARM_THRESHOLD) || 
    (incubator->avrTemp < incubator->setTemp - TEMP_ALARM_THRESHOLD))
  {
    *TemperatureAlarm = ALARM;
    return;
  }
}

void CheckHumidityAlarm(Incubator_t *incubator, Alarm_t *HumidityAlarm)
{
  /*Serial.println();
  Serial.println(*HumidityAlarm);
  Serial.println(incubator->setHumidity + HUMID_ALARM_THRESHOLD);
  Serial.println(incubator->chamberHumidity);
  Serial.println(incubator->setHumidity - HUMID_ALARM_THRESHOLD);*/

 //Si l'alarm est mute et que l'humidité est redevenue normale, on l'enlève
 if(*HumidityAlarm == ALARM_MUTE &&
  (incubator->chamberHumidity <= incubator->setHumidity + HUMID_ALARM_THRESHOLD) && 
    (incubator->chamberHumidity >= incubator->setHumidity - HUMID_ALARM_THRESHOLD)){
      *HumidityAlarm = NONE;
      //Serial.println("Humidity now ok, Alarm now at none");
      return;
    
 }

  //Si on est muté, on quitte
  if(*HumidityAlarm == ALARM_MUTE){
      //Serial.println("Alarm Muted, still in error");
      return;
  } 

  //Si l'humidité est trop haute ou basse, on met l'alarm.
  if((incubator->chamberHumidity > incubator->setHumidity + HUMID_ALARM_THRESHOLD) || 
    (incubator->chamberHumidity < incubator->setHumidity - HUMID_ALARM_THRESHOLD))
  {
    *HumidityAlarm = ALARM;
    //Serial.println("new error, go into alarm");
    return;
  }
}