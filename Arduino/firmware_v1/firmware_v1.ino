/*
 * CIRCUIT CONNECTIONS
 * todo  
 *  
 * I'm using an Arduino Micro
 * which has interrupts on pins 2, 3, 7
 * Note: On Arduino Micro pin 2 and 3 are on port D on position 1 and 0 respectively (see decoding snippet)
 * 
 * 2018-10-14 - MODS
 * Because we have a large magnet as reset,
 * and because of the fact that a code like this:
 * 
 * [...]
 * attachInterrupt(digitalPinToInterrupt(RESET), reset_ISR, CHANGE);
 * [...]
 * void reset_ISR()
 * {
 *   integrator = 0;
 *   if( bSendSerial ) { Serial.println("r;"); }
 * }
 * 
 * would have trigger the reset twice (one on first sight of the magnet,
 * the second other getting outside it), I have made some modification in order for the 
 * reset to happen only on one of these two margins.
 * 
 * In order to do this I was forced to read the status of the reset pin (register PINE position 6)
 * and behaving differently if we are moving CW or CCW.
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
float angle = 0.0, prevAngle =0.0;
volatile enum STATE {
  ENC_IDLE,
  ENC_CW,
  ENC_CCW
} status;

// we know the Bourns  encoder has 100 counts per shaft turn
// but we are incrementing the counter each time we have a new
// CHA and CHB configuration so 4 counts each quadrature cycle.
// This way we have 100 * 4 = 400 counts per shaft turn.
#define TOTALSTEPS 400

// Because magnet suspension harm is not perfectly aligned with the 
// axix on which the jewel, the main rotary disc, the Hall effect sensor 
// and the "snapper" mechanism are aligned to, we need to compensate 
// for this slight difference in alignements.
// I'm creating an offset expressed in term of (encoder steps * 4).
// INSTRUCTION:
// 1. set the stepoffeset to 0
// 2. uncomment the lines where you print information about the integrator and the new integrator (inside the loop)
//   if(DEBUG) {
//    Serial.print( integrator );
//    Serial.print(" - ");
//    Serial.print( newIntegrator );
//    Serial.print(" - ");
//    Serial.print( angle );
//    Serial.println();
//  }
// 3. then check the number on the first column.
//    Take note of it when, moving the wheel you are on the "Sviluppo Farmaceutico" (which is RED, index 0)
// 4. write down this number subtracting it from 400
//    So you will have the new integral saying 0.0 when you'll be on top of "Sviluppo Farmaceutico"
#define STEPOFFSET 400-2 //251
// console ricerca destra:   400-325
// console ricerca centro:   400-84
// console ricerca sinistra: 400-251

// While the main integrator is aligned with the actual position of the
// magned suspension harm, I also need a different integrator which takes
// into account the rotation due to the step offset.
// This new integrator is perfectly aligned with the above mentioned axis:
// it has its zero exaclty on this axis.
int newIntegrator = 0;
// Thank to that new integrator I can calculate with extreme precision
// an angle between 0.0 and 360.0 degrees which zero is perfectly aligned with
// the new integrator 0.


// LED STUFF
#include "Adafruit_NeoPixel.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define LEDPIN        9 // On Trinket or Gemma, suggest changing this to 1

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
volatile bool bReset = false;

// SLICE STUFF
#define NSLICES 7
#include "Slice.h" 
Slice slices[NSLICES];

// DEBUG STUFF
#define DEBUG false


// SETUP /////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);
  while(!Serial) {delay(1);};
  //Serial.println("here I'm");

  // ENCODER STUFF
  pinMode(CHA,   INPUT_PULLUP );
  pinMode(CHB,   INPUT_PULLUP );
  attachInterrupt(digitalPinToInterrupt(CHA),   ISR_chA,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(CHB),   ISR_chB,   CHANGE);
  
  // RESET STUFF
  // hall effect sensor VOUT goes LOW when facing the magnet
  attachInterrupt(digitalPinToInterrupt(RESET), ISR_RESET, CHANGE);


  float sliceSize = 360.0 / NSLICES; //degree
  for(int i=0; i<NSLICES; i++) 
  {
    float center = i*sliceSize;
    slices[i].init(i, sliceSize, center, 0.70, MAXBRIGHTNESS);
  }

  if( DEBUG ) {
    slices[0].setLight(&pixels, 255,   0,   0); // red    // "Sviluppo Farmaceutico"
    slices[1].setLight(&pixels, 0,   255,   0); // green  // "Ricerca Botanica"
    slices[2].setLight(&pixels, 0,     0, 255); // blue   // "Ricerca Clinica e Vigilanza"
    slices[3].setLight(&pixels, 255, 255,   0); // yellow // "Big Data Management"
    slices[4].setLight(&pixels, 255, 0,   255); // purple // "Ricerca In scienze Metaboliche"
    slices[5].setLight(&pixels,   0, 255, 255); // cyan   // "Ricerca Storica"
    slices[6].setLight(&pixels, 255, 255, 255); // white  // "Ricerca di Biologia dei Sistemi"
  }
  else
  {
    slices[0].setLight(&pixels, 255, 255, 255); // white
    slices[1].setLight(&pixels, 255, 255, 255); // white
    slices[2].setLight(&pixels, 255, 255, 255); // white
    slices[3].setLight(&pixels, 255, 255, 255); // white
    slices[4].setLight(&pixels, 255, 255, 255); // white
    slices[5].setLight(&pixels, 255, 255, 255); // white
    slices[6].setLight(&pixels, 255, 255, 255); // white
  }

  pixels.begin(); // Initialize NeoPixel strip object (REQUIRED)
  pixels.show();  // Initialize all pixels to 'off'
  
}


// LOOP //////////////////////////////////////////////////////////////
void loop() 
{
  // calculate the newIntegrator taking into account the total number of steps and the stepoffset
  newIntegrator = (integrator + STEPOFFSET)%TOTALSTEPS;
  // obtain the absolute angle from the newIntegrator
  angle = (360.0 * newIntegrator ) / TOTALSTEPS;

  if(DEBUG) {
    Serial.print( integrator );
    Serial.print(" - ");
    Serial.print( newIntegrator );
    Serial.print(" - ");
    Serial.print( angle );
    Serial.println();
  }   
  
  // update angle for each slice only if needed
  if( angle != prevAngle ) {
    for(int i=0; i<NSLICES; i++) 
      slices[i].setAngle( angle );
    prevAngle = angle;
  }
  /*
  if(DEBUG) {
    for(int i=0; i<NSLICES; i++) 
    {
      if( i == 0 )
        slices[i].debug();
    }
  }
  */
   

  for(int i=0; i<NSLICES; i++) 
  {
    slices[i].update();
  }
  pixels.show();

  delay(10);
}

// OTHER STUFF ///////////////////////////////////////////////////////
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
      //Serial.println((integrator + STEPOFFSET) %TOTALSTEPS);
    }
    else if( ( bCurrent == 1 && bPrevious == 0 ) ||
             ( bCurrent == 3 && bPrevious == 1 ) ||
             ( bCurrent == 2 && bPrevious == 3 ) ||
             ( bCurrent == 0 && bPrevious == 2 )
           )
    {
      status = ENC_CCW;
      integrator --;
      //Serial.println( (integrator + STEPOFFSET) %TOTALSTEPS);
    }
    bPrevious = bCurrent;
  }
  // Turn a bipolar signal between -400 and 400 
  // to a signal which is unipolar going from 0 to 400;
  if( integrator < 0)
    integrator = TOTALSTEPS + integrator;
}


void ISR_RESET()
{
  // Digital pin 7 on Arduino Leonardo is on PORT E register, position 6;
  bReset = (PINE >> 6) & B00000001;

  if(bReset && status==ENC_CW)
  {
    //Serial.println(" == CW RESET MAGNET MARGIN ====================================== ");
    integrator = 0;
  }
  else if(!bReset && status==ENC_CCW)
  {
    //Serial.println(" == CCW RESET MAGNET MARGIN ==================================== ");
    integrator = 0;
  }
}
