#ifndef _DRAWGRAPH_H_
#define _DRAWGRAPH_H_

  #include <Arduino.h>
  #include <Config.h>
  #include <TFT_eSPI.h> // Hardware-specific library

  void Draw_screen_background();
  void Draw_data(float Tset, float Hset, float Hnow, float Troom, float Hroom, int TBT, int uptime, float Ttop, float Tmid, float Tlow);
  void Init_Table(int TempTable[][TABLE_HEIGHT]);
  void Add_Value_Table(int TempTable[][TABLE_HEIGHT],float tempHigh, float tempMid, float tempLow);
  void Draw_Table(int TempTable[][TABLE_HEIGHT]);

#endif