#include <Wire.h>

#define BAUD_RATE 9600

void setup() {
  Serial.begin(BAUD_RATE);

  Serial.println("Initializing.");

  Wire.begin();

  InitGate();

  Serial.println("Initialized.");
}

void loop() {
  int gate = ReadGate();
  Serial.print(" Gate: ");
  Serial.print(gate);
  Serial.print(" In: ");
  Serial.println(IsBallInGate());
}
