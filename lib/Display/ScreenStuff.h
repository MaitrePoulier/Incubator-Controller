#ifndef _DRAWGRAPH_H_
#define _DRAWGRAPH_H_

  #include <Arduino.h>
  #include <Config.h>
  #include <TFT_eSPI.h> // Hardware-specific library
  #include "Free_Fonts.h" // Include the header file attached to this sketch

  void Draw_screen_background();
  void Draw_data(float , float , float , float ,  int , int , float ,float , float , float );
  void Init_Table(uint16_t TempTable[][TABLE_HEIGHT]);
  void Add_Value_Table(uint16_t TempTable[][TABLE_HEIGHT],float , float , float );
  void Display_UpTime(unsigned long );
  void Display_Refresh(unsigned long);
  void Draw_Table(uint16_t TempTable[][TABLE_HEIGHT]);

#endif