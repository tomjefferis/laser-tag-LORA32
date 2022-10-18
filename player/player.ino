#include <IRremote.hpp>
#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>

//LoRa Pins
#define ss 5
#define rst 14
#define dio0 2
bool connected = False;
int playerNumber;

void setup() {
  // initilize serial
  Serial.begin(115200);
  // initilize LoRa and IR
  LoRa.setPins(ss, rst, dio0);
  LoRa.begin(866E6);
  // sync word to only take commands from server
  LoRa.setSyncWord(0xF3);
  // connects to server with wifi mac 

  while (!connected)
  {
    //if timer interval has been reached
    LoRa.beginPacket();
    LoRa.print(WiFi.macAddress());
    LoRa.endPacket();

    
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      // read packet
      while (LoRa.available()) {
      playerNumber = LoRa.read();
      connected = True;
      }
    }
  }
  
  
}

void loop() {


}
