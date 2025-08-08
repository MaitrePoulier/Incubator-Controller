#include <ScreenStuff.h>
#include <TFT_eSPI.h> // Hardware-specific library
extern TFT_eSPI tft;
extern TFT_eSprite TempTable_Spr;

void Draw_screen_background()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.setTextDatum(BL_DATUM);


  tft.drawString("Temperature Setpoint:", 10,190);
  tft.drawString("Humidity Setpoint:", 10,210);

  tft.drawString("Now:", 200,190);
  tft.drawString("Now:", 200,210);


  tft.setTextFont(1);
  tft.drawString("Room temp:", 10,270);
  tft.drawString("Room Humidity:", 10,280);
  tft.drawString("Time b. tilt:", 10,290);
  tft.drawString("Up Time", 10,300);

  tft.drawRect(0,0,TABLE_WIDTH-1,TABLE_HEIGHT-1,TFT_WHITE);
}

void Draw_data(float Tset, float Hset, float Troom, float Hroom, int TBT, int uptime, float Hnow, float Ttop, float Tmid, float Tlow)
{
  tft.setTextSize(1);
  tft.setTextFont(1);
  tft.setTextPadding(6*2);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.drawFloat(Tset,1,150,190,2);
  tft.drawString("°C", 183, 190, 2);

  tft.drawFloat(Hset,1,150,210,2);
  tft.drawString("%", 183, 210, 2); 

  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawFloat((Ttop+Tmid+Tlow)/3,1,230,190,2);
  tft.drawString("°C", 263, 190, 2);
  tft.drawFloat(Hnow,1,230,210,2);
  tft.drawString("%", 263, 210, 2); 


  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawFloat(Troom,1,95,270);
  tft.drawString("°C", 125, 270);

  tft.drawFloat(Hroom,1,95,280);
  tft.drawString("%", 125, 280); 

  tft.setTextPadding(6*3);
  tft.drawNumber(TBT,95,290);
  tft.setTextPadding(6*2);
  tft.drawString("min", 125, 290); 
 

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(Ttop,2,390,40,4);
  tft.drawString("°C", 460, 40,4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawFloat(Tmid,2,390,90,4);
  tft.drawString("°C", 460, 90,4);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawFloat(Tlow,2,390,140,4);
  tft.drawString("°C", 460, 140,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

}


void Init_Table(uint16_t TempTable[][TABLE_HEIGHT])
{
  //initialise the tableau to '0'
  //We loose 2 pixel for the white rectangle around the graph
  for (int i=0; i < TABLE_WIDTH-2; i++)
  {
    for (int j = 0; j < TABLE_HEIGHT-2; j++)
    {
      TempTable[i][j] = 0;
    }
  }
}

void Add_Value_Table(uint16_t TempTable[][TABLE_HEIGHT],float tempHigh, float tempMid, float tempLow)
{
    int position;

    //We displace all the data inside TempTable by one position to de right
    for (int i = TABLE_WIDTH-4; i >= 0 ; i--) {
        for (int j = 0; j < TABLE_HEIGHT-2; j++) 
        {
            TempTable[i+1][j] = TempTable[i][j];
        }
    }
    //We erase the first line 
    for (int j = 0; j < TABLE_HEIGHT-2; j++) 
        {
            TempTable[0][j] = 0;
        }
    
   
    position = (int)((tempHigh - MIN_VALUE) / RESOLUTION);

    //We assure that we do not go out of the table
    if (position <= 0 || position >= TABLE_HEIGHT) position = 0;
    //puis on inverse pour que le point 0,0 soit en haut à droite
    position = -position + TABLE_HEIGHT-2;
    TempTable[0][position] = 1;

    position = (int)((tempMid - MIN_VALUE) / RESOLUTION);
    //We assure that we do not go out of the table
    if (position <= 0 || position >= TABLE_HEIGHT) position = 0;
    //puis on inverse pour que le point 0,0 soit en haut à droite
    position = -position + TABLE_HEIGHT-2;
    TempTable[0][position] = 2;

    position = (int)((tempLow - MIN_VALUE) / RESOLUTION);
    //We assure that we do not go out of the table
    if (position <= 0 || position >= TABLE_HEIGHT) position = 0;
    //puis on inverse pour que le point 0,0 soit en haut à droite
    position = -position + TABLE_HEIGHT-2;
    TempTable[0][position] = 3;



}

void Display_UpTime(unsigned long currentMillis)
{
    unsigned long totalSeconds = currentMillis / 1000;
    unsigned long days = totalSeconds / 86400; // Seconds in a day
    unsigned long hours = totalSeconds / 3600;
    unsigned long minutes = ((totalSeconds /60)%60); 
    unsigned long seconds = totalSeconds % 60; 

    tft.setTextSize(1);
    tft.setTextFont(1);
    tft.setTextPadding(6*2);
    tft.setTextDatum(BL_DATUM);

    tft.drawString("  d", 95, 300);
    tft.drawString("  h", 119, 300); 
    tft.drawString("  m", 143, 300); 
    tft.drawString("  s", 167, 300); 
    tft.drawNumber(days,95,300);
    tft.drawNumber(hours,119,300);
    tft.drawNumber(minutes,143,300);
    tft.drawNumber(seconds,167,300);


    Serial.print("Days: ");
    Serial.print(days);
    Serial.print(", Hours: ");
    Serial.print(hours);
    Serial.print(", Minutes: ");
    Serial.print(minutes);
    Serial.print(", Seconds: ");
    Serial.println(seconds);
}


void Display_Refresh(unsigned long RefreshTime)
{
    tft.setTextSize(1);
    tft.setTextFont(1);
    tft.setTextPadding(6*2);
    tft.setTextDatum(BL_DATUM);

    Serial.print(RefreshTime);
    Serial.println("ms");
    tft.drawString("    ms", 95, 310); 
    tft.drawNumber(RefreshTime,95,310);
    
}
  




void Draw_Table(uint16_t TempTable[][TABLE_HEIGHT])
{
    // Function to draw the temperature table on the TFT display
    // Assuming TempTable is a 2D array of size TABLE_WIDTH by TABLE_HEIGHT

    //We use the sprite and not directly the tft to save SPI communication
    for (int i = 0; i < TABLE_WIDTH-3; i++) {
        TempTable_Spr.drawLine(i+1,1,i+1,TABLE_HEIGHT-3,TFT_BLACK);//on efface la ligne d'un coup

        for (int j = 0; j < TABLE_HEIGHT-3; j++) {
            if(TempTable[i][j] == 1)
            {
                TempTable_Spr.drawPixel(i+1,j+1,TFT_RED);
            }
            else if(TempTable[i][j] == 2)
            {
                TempTable_Spr.drawPixel(i+1,j+1,TFT_GREEN);
            }
            else if(TempTable[i][j] == 3)
            {
                TempTable_Spr.drawPixel(i+1,j+1,TFT_BLUE);
            }
        
        }
    }
}

  void Display_Heater(float output)
  {

    tft.setTextDatum(TL_DATUM);
    if (output > MIN_PID_TIME_WIDTH + 0.01)
    {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawFloat(output,3, 405, 160,2);
        //tft.drawString("Heat On ", 390, 190,2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    else
    {
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString("Heat Off", 405, 160,2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
  }

void Display_Humidifier(bool Humidifier)
{
    tft.setTextDatum(TL_DATUM);
    if (Humidifier == true)
    {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Humidity On ", 405, 180,2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    else
    {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Humidity Off", 405, 180,2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
}

void Display_Fan(bool Fan)
{
    tft.setTextDatum(TL_DATUM);
    if (Fan == true)
    {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Fan On ", 405, 200,2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    else
    {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Fan Off", 405, 200,2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
}

void Display_Tilt(int Tilt)
{
    tft.setTextDatum(TL_DATUM);
    switch(Tilt)
    {
        case 0:
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString("Tilt Off  ", 405, 220,2);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            break;
        case 1:
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawString("Tilt Up   ", 405, 220,2);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            break;
        case 2:
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawString("Tilt Down", 405, 220,2);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            break;
    }
}