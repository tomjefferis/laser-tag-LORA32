#include <IRremote.hpp>
#include <LoRa.h>
#include <SPI.h>

//LoRa Pins
#define ss 5
#define rst 14
#define dio0 2

// on receive callback
void onLoraReceive(int packetSize) {
  // if there is data available
  if (packetSize) {
    // read packet
    String packet = "";
    while (LoRa.available()) {
      packet += (char)LoRa.read();
    }
    Serial.println(packet);
  }
}

void setup() {
  // initilize serial
  Serial.begin(115200);
  // initilize LoRa and IR
  LoRa.setPins(ss, rst, dio0);
  LoRa.begin(866E6);
  LoRa.setSyncWord(0xF3);
  LoRa.onReceive(onLoraReceive);
  Serial.println("LoRa Connected!");
}

void loop() {
  if(Serial.available() > 0) {
		char data = Serial.read();
		char str[2];
		str[0] = data;
		str[1] = '\0';
		
    LoRa.beginPacket();
    LoRa.print(str);
    LoRa.endPacket();
    LoRa.receive();
	}
}
