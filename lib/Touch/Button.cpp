#include "Button.h"

extern TFT_eSPI tft;
int TempState; //Vraiment pas élégant.. mais je ne sais pas comment faire autrement.

ButtonWidget btTup  = ButtonWidget(&tft);
ButtonWidget btTdown  = ButtonWidget(&tft);
ButtonWidget btHup  = ButtonWidget(&tft);
ButtonWidget btHdown = ButtonWidget(&tft);
ButtonWidget btTiltup = ButtonWidget(&tft);
ButtonWidget btTiltdown = ButtonWidget(&tft);
ButtonWidget btAlarm  = ButtonWidget(&tft);


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


// Create an array of button instances to use in for() loops
// This is more useful where large numbers of buttons are employed
ButtonWidget* btn[] = {&btTup , &btTdown, &btHup, &btHdown, &btTiltup, &btTiltdown, &btAlarm};
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

int MainScreenButton(bool pressed,u_int16_t x,u_int16_t y)
{
  tft.setFreeFont(FF17);
  tft.setTextSize(1);
  tft.setTextPadding(9*4);
  tft.setTextDatum(TR_DATUM);

  TempState = 0;
  static uint32_t scanTime = millis();

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    scanTime = millis();
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (pressed) {
        if (btn[b]->contains(x, y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
return TempState;
}



void btTup_pressAction(void)
{
  if (btTup.justPressed()) {
    //Serial.println("Left button just pressed");
    btTup.drawSmoothButton(true);
    TempState = 1; //temp +0.1oC
  }
}

void btTup_releaseAction(void)
{
  if (btTup.justReleased()) {
    //Serial.println("Left button just released");
    btTup.drawSmoothButton(false);
  }
  
}

void btTdown_pressAction(void)
{
  if (btTdown.justPressed()) {
    //Serial.println("Left button just pressed");
    btTdown.drawSmoothButton(true);
    TempState = 2; //temp -0.1oC
  }
}

void btTdown_releaseAction(void)
{
  if (btTdown.justReleased()) {
    //Serial.println("Pesée");
    btTdown.drawSmoothButton(false);
  }
}

void btHup_pressAction(void)
{
  if (btHup.justPressed()) {
    btHup.drawSmoothButton(true);
  }
  TempState = 3; //Humidity +1%
}

void btHup_releaseAction(void)
{
  if (btHup.justReleased()) {
    //Serial.println("Qualib");
    btHup.drawSmoothButton(false);
  }
}

void btHdown_pressAction(void)
{
  if (btHdown.justPressed()) {
    btHdown.drawSmoothButton(true);
  }
  TempState = 4; //Humidity -1%
}

void btHdown_releaseAction(void)
{
  if (btHdown.justReleased()) {
    //Serial.println("Qualib");
    btHdown.drawSmoothButton(false);
  }
}


void btTiltup_pressAction(void)
{
  if (btTiltup.justPressed()) {
    btTiltup.drawSmoothButton(true);
    TempState = 5; //Tilt UP until release
  }
}

void btTiltup_releaseAction(void)
{
  if (btTiltup.justReleased()) {
    //Serial.println("Qualib");
    btTiltup.drawSmoothButton(false);
    TempState = 6; //Stop Tilt
  }
}

void btTiltdown_pressAction(void)
{
  if (btTiltdown.justPressed()) {
    btTiltdown.drawSmoothButton(true);
    TempState = 7; //Tilt Down until release
  }
}

void btTiltdown_releaseAction(void)
{
  if (btTiltdown.justReleased()) {
    //Serial.println("Qualib");
    btTiltdown.drawSmoothButton(false);
    TempState = 6; //Stop Tilt
  }
}

void btAlarm_pressAction(void)
{
  if (btAlarm.justPressed()) {
    btAlarm.drawSmoothButton(true);
    TempState = 8; //Will Mute alarm if on
  }
}

void btAlarm_releaseAction(void)
{
  if (btAlarm.justReleased()) {
    btAlarm.drawSmoothButton(false);
  }
}


void initButtons() {
  //For the button
  tft.setFreeFont(FF17);
  tft.setTextSize(1);
  tft.setTextPadding(9*4);
  tft.setTextDatum(TR_DATUM);
  // Calibrate the touch screen and retrieve the scaling factors
  //touch_calibrate();

  uint16_t y = 220;

  uint16_t x = 190;
  btTup.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_PINK, TFT_BLACK, const_cast<char *>("T Up"), 1);
  btTup.setPressAction(btTup_pressAction);
  btTup.setReleaseAction(btTup_releaseAction);
  btTup.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  x = 260;
  btHup.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_PINK, TFT_BLACK, const_cast<char *>("H up."), 1);
  btHup.setPressAction(btHup_pressAction);
  btHup.setReleaseAction(btHup_releaseAction);
  btHup.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  x = 330;
  btTiltup.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_PINK, TFT_BLACK, const_cast<char *>("Ti Up"), 1);
  btTiltup.setPressAction(btTiltup_pressAction);
  btTiltup.setReleaseAction(btTiltup_releaseAction);
  btTiltup.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing


  y = 270;
  
  x = 190;
  btTdown.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_CYAN, TFT_BLACK, const_cast<char *>("T down"), 1);
  btTdown.setPressAction(btTdown_pressAction);
  btTdown.setReleaseAction(btTdown_releaseAction);
  btTdown.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  x = 260;
  btHdown.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_CYAN, TFT_BLACK, const_cast<char *>("H down"), 1);
  btHdown.setPressAction(btHdown_pressAction);
  btHdown.setReleaseAction(btHdown_releaseAction);
  btHdown.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  x = 330;
  btTiltdown.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_CYAN, TFT_BLACK, const_cast<char *>("Ti Down"), 1);
  btTiltdown.setPressAction(btTiltdown_pressAction);
  btTiltdown.setReleaseAction(btTiltdown_releaseAction);
  btTiltdown.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  
  x = 410;
  btAlarm.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_YELLOW, TFT_BLACK, const_cast<char *>("Alarm"), 1);
  btAlarm.setPressAction(btAlarm_pressAction);
  btAlarm.setReleaseAction(btAlarm_releaseAction);
  btAlarm.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  //We put beack the display setting
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.setTextDatum(BL_DATUM);
}

void displayAlarm(enum Alarm HumidityAlarm, enum Alarm TemperatureAlarm)
{
  if (HumidityAlarm != NONE || TemperatureAlarm != NONE )
  {
    if (btAlarm.getState() == 1) btAlarm.drawSmoothButton(false,3,TFT_BLACK,"Alarm");
    else  btAlarm.drawSmoothButton(true,3,TFT_YELLOW,"!!ALARM!!");  
  }
}


