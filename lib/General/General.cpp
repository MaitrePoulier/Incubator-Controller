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