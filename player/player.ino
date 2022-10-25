#include <IRremote.hpp>
#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>

// LoRa Pins
#define ss 5
#define rst 14
#define dio0 2
// player connected to server
bool connected = false;
// number and team of player
int playerNumber;
int teamNumber;
//hw timers for sending lora connection packets
hw_timer_t *Lora_Timer = NULL;
// player hit?
bool hit = false;

// ISR for pinging Lora server to connect
void IRAM_ATTR loraTimer(){
    LoRa.beginPacket();
    LoRa.print(WiFi.macAddress());
    LoRa.endPacket();
}

void setup() {
  // initilize serial
  Serial.begin(115200);
  // initilize LoRa and IR
  LoRa.setPins(ss, rst, dio0);
  // european lora
  LoRa.begin(866E6);
  // sync word to only take commands from server
  LoRa.setSyncWord(0xF3);
  // connects to server with wifi mac 

  Lora_Timer = timerBegin(0, 80, true);
  timerAttachInterrupt(Lora_Timer, &loraTimer, true);
  timerAlarmWrite(Lora_Timer, 10000000, true);
  timerAlarmEnable(Lora_Timer);  
  
}

void loop() {
    while (!connected)
  {
    //if timer interval has been reached
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      // read packet
      while (LoRa.available()) {
      String playerInfo = LoRa.readString();
      String mac = getValue(playerInfo,' ',0);

      if(mac == WiFi.macAddress())
        {
          playerNumber = getValue(playerInfo,' ',1).toInt();
          teamNumber = getValue(playerInfo,' ',2).toInt();  
          connected = true;
          timerAlarmDisable(Lora_Timer);
        }
      }
    }
  }
}


void startGame() {

}

void playGame() {
  
}


void endGame() {

}

void fire() {

}


// used for Lora Setup
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
