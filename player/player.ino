#include <IRremote.hpp>
#include <LoRa.h>



void setup() {
  // initilize serial
  Serial.begin(9600);
}

void loop() {

  Serial.println(analogRead(A1));
  delay(1);
}