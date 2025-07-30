#include <TFT_eSPI.h> // Hardware-specific library
#include <Config.h>
extern TFT_eSPI tft;

void Draw_screen_background()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.setTextDatum(BL_DATUM);
  //tft.setTextPadding(6*20);


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

void Draw_data(float Tset, float Hset, float Hnow, float Troom, float Hroom, int TBT, int uptime, float Ttop, float Tmid, float Tlow)
{
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.drawFloat(Tset,1,150,190,2);
  tft.drawString("°C", 183, 190, 2);

  tft.drawFloat(Hset,1,150,210,2);
  tft.drawString("%", 183, 210, 2); 

  tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.drawFloat((Ttop+Tmid+Tlow)/3,1,230,190,2);
  tft.drawString("°C", 263, 190, 2);

  tft.drawFloat(Hnow,1,230,210,2);
  tft.drawString("%", 263, 210, 2); 

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawFloat(Troom,1,95,270);
  tft.drawString("°C", 125, 270);

  tft.drawFloat(Hroom,1,95,280);
  tft.drawString("%", 125, 280); 

  tft.drawNumber(TBT,95,290);
  tft.drawString("min", 125, 290); 

  tft.drawNumber(uptime,95,300);
  tft.drawString("???", 125, 300); 

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


void Init_Table(int TempTable[][TABLE_HEIGHT])
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

void Add_Value_Table(int TempTable[][TABLE_HEIGHT],float tempHigh, float tempMid, float tempLow)
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
    //puis on inverse pour que le point 0,0 soit en haut à droite
    position = -position + TABLE_HEIGHT-2;
    TempTable[0][position] = 1;

    position = (int)((tempMid - MIN_VALUE) / RESOLUTION);
    //puis on inverse pour que le point 0,0 soit en haut à droite
    position = -position + TABLE_HEIGHT-2;
    TempTable[0][position] = 2;

    position = (int)((tempLow - MIN_VALUE) / RESOLUTION);
    //puis on inverse pour que le point 0,0 soit en haut à droite
    position = -position + TABLE_HEIGHT-2;
    TempTable[0][position] = 3;



}


void Draw_Table(int TempTable[][TABLE_HEIGHT])
{
    // Function to draw the temperature table on the TFT display
    // Assuming TempTable is a 2D array of size TABLE_WIDTH by TABLE_HEIGHT
    for (int i = 0; i < TABLE_WIDTH-3; i++) {
        for (int j = 0; j < TABLE_HEIGHT-3; j++) {
            if(TempTable[i][j] == 1)
            {
                tft.drawPixel(i+1,j+1,TFT_RED);
            }
            else if(TempTable[i][j] == 2)
            {
                tft.drawPixel(i+1,j+1,TFT_GREEN);
            }
            else if(TempTable[i][j] == 3)
            {
                tft.drawPixel(i+1,j+1,TFT_BLUE);
            }
            else
            {
                 tft.drawPixel(i+1,j+1,TFT_BLACK);   
            }
        
        }
    }
}