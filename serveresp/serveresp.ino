#include <IRremote.hpp>
#include <LoRa.h>
#include <SPI.h>

//LoRa Pins
#define ss 5
#define rst 14
#define dio0 2

void setup() {
  // initilize serial
  Serial.begin(115200);
  // initilize LoRa and IR
  LoRa.setPins(ss, rst, dio0);
  LoRa.begin(866E6);
  LoRa.setSyncWord(0xF3);
  
  
}

void loop() {


}
