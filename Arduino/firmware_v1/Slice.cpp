#include "Slice.h"

void Slice::init(int _index, float _sliceSize, float _center, float _focusSpread, float _maxBrightness)
{
  
	index = _index;
  sliceSize = _sliceSize;
	center = _center;
  borderLeft  = center - (sliceSize/2.0);
  borderRight = center + (sliceSize/2.0);
  
  focusSpread = _focusSpread;
  focusLeft  = center - (focusSpread * sliceSize * 0.5);
  focusRight = center + (focusSpread * sliceSize * 0.5);

  maxBrightness = _maxBrightness;
  sine.init(0.5, 0.0);


  // a fact: the angle coming from outside ranges from 0 to 360 degrees
  // There's a case in which a slice has its focusLeft and its borderLeft that 
  // are negative. This means that with this angle range 
  // we will never enter some of the if code blocks below.
  //
  // We should find a way to consider also this eventuality. 
  if( borderLeft < 0 || focusLeft <0 )
    bNegativeLeft = true;
  else
    bNegativeLeft = false;
 

  
  Serial.print("Slice ");
  Serial.print(index);
  Serial.print("] ");
  Serial.print(sliceSize);
  Serial.print(" - \t");
  Serial.print(center);
  Serial.print(" [");
  Serial.print(borderLeft);
  Serial.print(", ");
  Serial.print(borderRight);
  Serial.print("] ");
  Serial.print("\t[");
  Serial.print(focusLeft);
  Serial.print(", ");
  Serial.print(focusRight);
  Serial.print("] ");
  Serial.print(" bNegativeBorder: ");
  Serial.print(bNegativeLeft);
  Serial.println();
  
}

void Slice::setLight(Adafruit_NeoPixel* _strip, int _r, int _g, int _b)
{
  strip = _strip;
  NLEDS = strip->numPixels();
  RED   = _r;
  GREEN = _g;
  BLUE  = _b;

  /*
  Serial.print("numLeds: ");
  Serial.print(NLEDS);
  Serial.println();
  */
}

void Slice::setAngle(float _angle )
{
  if( bNegativeLeft && _angle-360.0 > borderLeft && _angle - 360.0 < center)
    angle = _angle - 360.0;
  else
    angle = _angle;
    
  if( angle != prevAngle ) 
  {
  	if(angle > focusLeft && angle <= focusRight)
  	{
  		if( mainStatus != ONFOCUS )
      {
  			mainStatus = ONFOCUS;
        secondaryStatus = WAIT;
        startTime = millis();
        waitTime = TIMETOBESHURETOBEINFOCUS;
      } 
  	}
  	else if(angle > borderLeft && angle <= borderRight)
  	{
  		if( mainStatus != ONLOBES ) {
  		 mainStatus = ONLOBES;
       secondaryStatus = WAIT;
  		}       
  	}
    else
    {
      if( mainStatus != OUTSIDE ) {
        mainStatus = OUTSIDE;
        secondaryStatus = WAIT;
      }
    }
  }
  prevAngle = angle;
}

void Slice::update()
{
	if( mainStatus == ONFOCUS )
	{
    // we are on focus, we are snapped to a particular slice now.
    // So the thing we have to do now is to discern some secondary status:
    if( secondaryStatus == WAIT )
    {
      if( millis()-startTime< waitTime ) {
        // we are in wait
        // se we maintain the LED to theyr maximum brightness
        y = 1.0;
      }
      else
      {
        secondaryStatus = BLINK;
        startTime = millis();
        waitTime  = TIMETOBESHURETOBEINFOCUS; // not useful in the next status
      }
    }
    else if( secondaryStatus == BLINK )
    {
      if(millis() - startTime < blinkHalfTime)
      {
        // first half of the blink
        y = 1.0;
      }
      else if(millis() - startTime < (blinkHalfTime*2)) {
        // second half of the blink half of the blink
        y = 0.0;
      }
      else 
      {
        // every complete blink cycle
        // reset the initial time of blinking 
        startTime = millis();
        blinkCounter ++;
        
        if(blinkCounter == N_BLINK) 
        {
          // if we are here the blinking cycle has been completed
          blinkCounter = 0;
          secondaryStatus = STAY;
          startTime = millis();
          waitTime = TIMETOMOVEFROMWAITTOSTANDBY;
          Serial.print("siamo un focus sulla fetta ");
          Serial.print(index);
          Serial.println();
        }
      }
      
    }
    else if( secondaryStatus == STAY )
    {
      if( millis()-startTime< waitTime ) {
        // we are in wait
        // se we maintain the LED to theyr maximum brightness
        y = 1.0;
      }
      else
      {
        secondaryStatus = STANDBY;
        startTime = millis();
        waitTime  = 3000; // not useful in the next status
      }
    }
    else if(secondaryStatus == STANDBY )
    {
      sine.update();
      y = sine.getY();
    }
    for(int i=0; i<NLEDS; i++)
    {
      strip->setPixelColor(i, y*RED*maxBrightness, y*GREEN*maxBrightness, y*BLUE*maxBrightness);
    }
	}
  else if( mainStatus == ONLOBES ) 
  {
    distance = abs(center - angle);
    //y = map( distance, abs(center-borderLeft), abs(center-focusLeft), 0.0, 1.0);
    y = (1 / (abs(center-focusLeft) - abs(center-borderLeft))) * (distance - abs(center-borderLeft));
    for(int i=0; i<NLEDS; i++)
    {
      strip->setPixelColor(i, y*RED*maxBrightness, y*GREEN*maxBrightness, y*BLUE*maxBrightness);
    }
  }
  else
	{
    y = 0.0;
    // do nothing because in case the "black"
    // color of the OUTSIDE status for the Slice
    // will hide colors for the other ones
    /*
    
		for(int i=0; i<NLEDS; i++)
    {
      strip->setPixelColor(i, y*RED, y*GREEN, y*BLUE);
    }
    */
	}
}

void Slice::debug()
{
	Serial.print("Slice: ");
	Serial.print(index);
	Serial.print(", ");
  Serial.print("\tangle: ");
  Serial.print( angle );
	
  Serial.print("\tdistance: ");
  Serial.print(distance);

  printStatus();
  /*
  Serial.print("\ty: ");
  Serial.print(y);
  Serial.print("\tcenter-borderLeft: ");
  Serial.print( abs(center-borderLeft) );
  Serial.print("\tcenter-focusLeft: ");
  Serial.print( abs(center-focusLeft) );
  */
	Serial.println();
}

void Slice::printStatus()
{
  Serial.print(" [");
  switch( mainStatus ) {
    case 0:
      Serial.print( "BEGINNING" );
      break;
    case 1:
      Serial.print( "ONFOCUS" );
      break;
    case 2:
      Serial.print( "ONLOBES" );
      break;
    case 3:
      Serial.print( "OUTISIDE" );
      break;
    default:
      break;
  }

  Serial.print(" - ");
  
  switch( secondaryStatus ) {
    case 0:
      Serial.print( "WAIT" );
      break;
    case 1:
      Serial.print( "BLINK" );
      break;
    case 2:
      Serial.print( "STAY" );
      break;
    case 3:
      Serial.print( "STANDBY" );
      break;
    default:
      break;
  }

  Serial.print(" ] ");
}
