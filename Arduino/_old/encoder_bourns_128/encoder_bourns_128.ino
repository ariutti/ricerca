#define A 2
#define B 3


// /////////////
// ENCODER STUFF 
// /////////////

// Encoder code is inspired by the work of eran.io
// http://eran.io/rotary-encoder-based-cooking-timer/
// Here's code: https://github.com/pavius/rotary-encoder-timer

#include "eran_encoder.h"
long m1_counter = 0;

EranEncoder m1_encoder; 

void m1_cwStep(void) {
  m1_counter ++;
  //m1_status = SEARCH;
  //m1_prevTime = millis();
  Serial.println(m1_counter);
}

void m1_ccwStep(void) {
  m1_counter --;
  //m1_status = SEARCH;
  //m1_prevTime = millis();
  Serial.println(m1_counter);
}


void setup() {
  Serial.begin(9600);
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  
  m1_encoder.init(A, B);
  m1_encoder.setCallbackCW(m1_cwStep);
  m1_encoder.setCallbackCCW(m1_ccwStep);
}

void loop() {
  m1_encoder.update();

}
