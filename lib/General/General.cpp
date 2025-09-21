#include <General.h>

void SentSerial(float room_h, float chamber_h,float room_t, float avrTemp)
{
  Serial.print(F("Room Humidity: "));
  Serial.print(room_h);
  Serial.println(F("%"));

  Serial.print(F("Chamber  Humidity: "));
  Serial.print(chamber_h);
  Serial.println(F("%"));

  Serial.print("Room Temperature: ");
  Serial.print(room_t);
  Serial.println(F("oC"));

  Serial.print("avrg. temps = "); 
  Serial.println(avrTemp);
}