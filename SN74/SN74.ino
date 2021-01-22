
int dataPin = 15;
int latchPin = 0;
int clockPin = 4;
int nummer[10] = {63,6,91,79,102,109,125,7,127,111};
void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Serial.begin(115200);
  
}

void loop() {
for (int i=0; i < 10; i++){
  // take the latchPin low so
  // the LEDs don't change while you're sending in bits:
  digitalWrite(latchPin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, nummer[i]);
  Serial.println(nummer[i]);
  //take the latch pin high so the LEDs will light up:
  digitalWrite(latchPin, HIGH);
  // pause before next value:
  delay(200);
}
}
