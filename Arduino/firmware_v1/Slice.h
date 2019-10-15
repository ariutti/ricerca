/*
  Slice
  Nicola Ariutti
  2019/10/07
*/

#ifndef SLICE_H
#define SLICE_H

#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "Animator_Sine.h"

class Slice
{
  private:
    int index;
    // this is the value in degrees of the size of a slice
    // inside the complete circle.
    float sliceSize;
    float center;
    float borderLeft;
    float borderRight;
    
    // focus spread this is a value expressed in percentage
    // on the sliceSize 
    float focusSpread; 
    float focusLeft;
    float focusRight;

    boolean bNegativeLeft = false;
    
    // the moving angle, set from outside by the encode movement
    float angle;
    float prevAngle = -180.0;
    Adafruit_NeoPixel* strip;
    int NLEDS;
    int RED, GREEN, BLUE;
    float maxBrightness;
    float y = 0.0;
    float distance = 0.0;

    Animator_Sine sine;
    
  enum MAINSTATE {
  	BEGINNING=0,
    ONFOCUS,
    ONLOBES,
    OUTSIDE,
  } mainStatus = BEGINNING;

  enum {
    WAIT=0,
    BLINK,
    STAY,
    STANDBY
  } secondaryStatus = STANDBY;

  // TIME and TIME related stuff
  long startTime;
  long waitTime;
  int N_BLINK = 6;
  long blinkHalfTime = 250;
  int blinkCounter = 0;
  int TIMETOBESHURETOBEINFOCUS = 1000;
  int TIMETOMOVEFROMWAITTOSTANDBY = 2000;
   

  public:
    Slice() {};
    ~Slice() {};

    void init(int _index, float _sliceSize, float _center, float _focusSpread, float _maxBrightness);
    // to be called just after the initialization
    void setLight(Adafruit_NeoPixel* strip, int _r, int _g, int _b);
    void update();
		void setAngle(float _angle);
    void debug();
    void printStatus();

    //void changeFreq(float _freq);
    //float getY();
};

#endif
