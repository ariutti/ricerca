/*
 *  CIRCUIT CONNECTIONS
 *  
 *  TODO
 *  
 * I'm using an Arduino Micro
 * which has interrupts on pins 2, 3, 7
 * Note: On Arduino Micro pin 2 and 3 are on port D on position 1 and 0 respectively (see decoding snippet)
 *
*/


// ENCODER STUFF

// On Arduino Micro digital pins 2 and 3 (interrupt # 1 and 0 respectively)
// are on the same port (which is port D). These pins are on position 1 and 0 of the port
#define CHA 2 
#define CHB 3 
volatile byte bCurrent = 0;
volatile byte bPrevious = 0;
volatile long integrator = 0;
volatile long prevIntegrator = 0;
float angle = 0;
volatile enum STATE {
  ENC_IDLE,
  ENC_CW,
  ENC_CCW
} status;

// we know the Bourns  encoder has 100 counts per shaft
// but we are incrementing the counter each time we have a new
// CHA and CHB configuration so 4 counts each quadrature cycle.
// This way we have 100 * 4 = 400 counts per shaft turn.
#define TOTALSTEPS 400


// LED STUFF
#include "Adafruit_NeoPixel.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define LEDPIN        4 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 7 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
#define MAXBRIGHTNESS 0.2


// RESET STUFF
#define RESET 7

// SLICE STUFF
#define NSLICES 7
#include "Slice.h" 
Slice slices[NSLICES];


// SETUP /////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);
  while(!Serial) {delay(1);};

  // ENCODER STUFF
  pinMode(CHA,   INPUT_PULLUP );
  pinMode(CHB,   INPUT_PULLUP );
  attachInterrupt(digitalPinToInterrupt(CHA),   ISR_chA,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(CHB),   ISR_chB,   CHANGE);
  
  // RESET STUFF
  // hall effect sensor VOUT goes LOW when facing the magnet
  attachInterrupt(digitalPinToInterrupt(RESET), ISR_RESET, FALLING);


  float sliceSize = 360.0 / NSLICES;
  for(int i=0; i<NSLICES; i++) 
  {
    float center = i*sliceSize;
    slices[i].init(i, sliceSize, center, 0.33, MAXBRIGHTNESS);
  }

  slices[0].setLight(&pixels, 255,   0,   0); // red
  slices[1].setLight(&pixels, 0,   255,   0); // green
  slices[2].setLight(&pixels, 0,     0, 255); // blue
  slices[3].setLight(&pixels, 255, 255,   0); // yellow
  slices[4].setLight(&pixels, 255, 0,   255); // purple
  slices[5].setLight(&pixels,   0, 255, 255); // cyan
  slices[6].setLight(&pixels, 255, 255, 255); // white

  pixels.begin(); // Initialize NeoPixel strip object (REQUIRED)
  pixels.show();  // Initialize all pixels to 'off'
  
}


// LOOP //////////////////////////////////////////////////////////////
void loop() 
{
  //m1_encoder.update();
  angle = (360.0 * integrator) / TOTALSTEPS;

  /*
  Serial.print( integrator );
  Serial.print(" - ");
  Serial.print( angle );
  Serial.println();
  */

  for(int i=0; i<NSLICES; i++) 
  {
    slices[i].setAngle( angle );
  }

  for(int i=0; i<NSLICES; i++) 
  {
    slices[i].update();
  }
  pixels.show();

  /*
  Serial.print( angle );
  Serial.println();
  slices[0].printStatus();
  */

  delay(10);
}

// OTHER STUFF ///////////////////////////////////////////////////////
void printStatuses()
{
  for(int i=0; i<NSLICES; i++) 
  {
    slices[i].printStatus();
  }
  Serial.println();
}



void ISR_chA()
{
  //Serial.print("a");
  bCurrent = (PIND & B00000011)>>0;
  //bCurrent = (PINE & B00110000)>>4;
  //Serial.println(bCurrent);
  decode();
}

void ISR_chB()
{
  //Serial.print("b");
  bCurrent = (PIND & B00000011)>>0;
  //bCurrent = (PINE & B00110000)>>4;
  //Serial.println(bCurrent);
  decode();
}

/*
 * Questa funzione incrementa la variabile 'volatile'
 * ogni nuovo passaggio tra le quadrature.
 */
void decode()
{
  //Serial.println(bCurrent);
  prevIntegrator = integrator;

  // encoder stuff
  if( bCurrent != bPrevious )
  {
    if( ( bCurrent == 2 && bPrevious == 0 ) ||
        ( bCurrent == 3 && bPrevious == 2 ) ||
        ( bCurrent == 1 && bPrevious == 3 ) ||
        ( bCurrent == 0 && bPrevious == 1 )
      )
    {
      status = ENC_CW;
      integrator ++;
      //Serial.println(integrator);
    }
    else if( ( bCurrent == 1 && bPrevious == 0 ) ||
             ( bCurrent == 3 && bPrevious == 1 ) ||
             ( bCurrent == 2 && bPrevious == 3 ) ||
             ( bCurrent == 0 && bPrevious == 2 )
           )
    {
      status = ENC_CCW;
      integrator --;
      //Serial.println(integrator);
    }
    bPrevious = bCurrent;
  }
}

void ISR_RESET()
{
  //Serial.println("reset");
  integrator = 0;
}
