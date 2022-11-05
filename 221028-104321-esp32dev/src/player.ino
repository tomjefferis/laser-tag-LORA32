#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <NeoPixelBus.h>

// LoRa Pins
#define ss 5
#define rst 14
#define dio0 2
// player connected to server
bool connected = false;
// number and team of player
int playerNumber = 0;
int teamNumber = 0;
// hw timers for sending lora connection packets
hw_timer_t *Lora_Timer = NULL;
// hw timer for counting hit time 
hw_timer_t *Hit_Timer = NULL;
// player hit?
bool hit = false;
bool loraHit = false;
bool masterHit = false;
// trigger button pin
int triggerPin = 35;
// IR receiver pin
const uint16_t  IRpin = 15;
// IR receiver object
IRrecv irrecv(IRpin);
// IR emitter pin
const uint16_t  IRemitterPin = 4;
// IR emitter object
IRsend irsend(IRemitterPin);
// setup for led strip

// ISR for pinging Lora server to connect
void IRAM_ATTR loraTimer()
{
  LoRa.beginPacket();
  LoRa.print(WiFi.macAddress());
  LoRa.endPacket();
  // put the radio into receive mode
  LoRa.receive();
}

// ISR for shooting IR signal when trigger is pressed
void IRAM_ATTR triggerISR()
{
  //if masterhit is false
  if(!masterHit)
  {
    // send ir signal with player number and team number
    int send = concatenate(playerNumber, teamNumber);
    irsend.sendNEC(send, 16);
  }
}

// ISR for receiving IR signal when hit
void IRAM_ATTR hitISR()
{
  //if master hit is false
  if(!masterHit)
  {
  //read IR signal
  decode_results results;
  irrecv.decode(&results);
  // set hit to true
  hit = true;
  // send lora hit signal
  sendHit(results.value);
  }
}

// isr for resetting hit after 10 seconds
void IRAM_ATTR resetHit()
{
  hit = false;
  loraHit = false;
  masterHit = false;
}

void setup()
{
  // initilize serial
  Serial.begin(115200);
  // initilize LoRa and IR
  LoRa.setPins(ss, rst, dio0);
  // european lora
  LoRa.begin(866E6);
  // sync word to only take commands from server
  LoRa.setSyncWord(0xF3);
  // connects to server with wifi mac
  // attach timer interrupt
  Lora_Timer = timerBegin(0, 80, true);
  timerAttachInterrupt(Lora_Timer, &loraTimer, true);
  timerAlarmWrite(Lora_Timer, 10000000, true);
  timerAlarmEnable(Lora_Timer);
  // attach hit timer interrupt
  Hit_Timer = timerBegin(1, 80, true);
  timerAttachInterrupt(Hit_Timer, &resetHit, true);
  timerAlarmWrite(Hit_Timer, 10000000, true);
  // lora on recieve
  LoRa.onReceive(onLoraReceive);
}

void loop()
{
}

void onLoraReceive(int packetSize)
{
  // if timer interval has been reached
  if (packetSize)
  {
    // read packet
    while (LoRa.available())
    {
      // get length of packet and if less than 20 bytes
      if (packetSize < 20)
      {
        String playerInfo = LoRa.readString();
        String mac = getValue(playerInfo, ' ', 0);
        if (!connected && mac == WiFi.macAddress())
          {
            loraConnect(playerInfo);
          }
      }
      else
      {
        // if packet contains "start" start game
        String condition = LoRa.readString();
        if (condition == "tag-start")
        {
          // start game
          startGame();
        }
        else if (condition == "tag-stop")
        {
          // stop game
          endGame();
        }
        else if (condition.indexOf("player-hit") > 0 && condition.indexOf(WiFi.macAddress()) > 0)
        {
          // player has been hit
          loraHit = true;
          // if hit is true and lora hit is true and master hit is true
          if (hit && loraHit)
          {
            masterHit = true;
            //enable hit timer
            timerAlarmEnable(Hit_Timer);
          }
        }
      }
    }
  }
}

// function to send hit to lora server
void sendHit(int results)
{
  LoRa.beginPacket();
  LoRa.print(strconcat(6,"player-shot ", playerNumber, " ",teamNumber, " hit by ", results));
  LoRa.endPacket();
  LoRa.receive();
}

void loraConnect(String playerInfo)
{
  playerNumber = getValue(playerInfo, ' ', 1).toInt();
  teamNumber = getValue(playerInfo, ' ', 2).toInt();
  connected = true;
  timerAlarmDisable(Lora_Timer);
}

void startGame()
{
  //add isr to trigger hit 
  attachInterrupt(digitalPinToInterrupt(IRpin), hitISR, RISING);
  //add pin interupt on ir reciever
  irrecv.enableIRIn();
  // isr to send hit on trigger pin
  attachInterrupt(digitalPinToInterrupt(triggerPin), triggerISR, RISING);
}


void endGame()
{
  //disable ir reciever
  irrecv.disableIRIn();
  //disable trigger isr
  detachInterrupt(digitalPinToInterrupt(triggerPin));
  //disable hit isr
  detachInterrupt(digitalPinToInterrupt(IRpin));
}


// used for Lora Setup
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

unsigned concatenate(unsigned x, unsigned y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}

char* strconcat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = (char*) calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}
