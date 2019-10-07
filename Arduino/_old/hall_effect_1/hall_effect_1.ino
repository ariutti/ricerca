#define HALL 7

void setup() {
  Serial.begin(9600);
  pinMode(HALL, INPUT_PULLUP);
}

int value = 0;

void loop() {
  value = digitalRead(HALL);
  Serial.println( value );
  delay(10);
}
