#include "PoliteSerial.h"
#define RCBLEN 16

char rcb [RCBLEN];
unsigned char wrHead=0;
unsigned char readHead=0;
// #define RX1PIN 0
// #define TX1PIN 1
bool outgoDue=false;

#define ECHOLATENCY 5000
long messageReceivedAt=0;

unsigned char outgoingQueue [MSGLEN];
unsigned char incomingQueue[MSGLEN];
PoliteSerial politeSerial(&Serial1,19,18,9600);

void setup(){
  politeSerial.init();
  politeSerial.onMessage(onPoliteMessage);
  DDRB=0xff;
}

void loop(){
  PORTB=politeSerial.loop();
  if(outgoDue){
    if(millis()-messageReceivedAt>ECHOLATENCY){
      sendMessage();
      outgoDue=false;
    }
  }

}
void onPoliteMessage(){
  messageReceivedAt=millis();
  for(unsigned char a=0; a<MSGLEN; a++){
    outgoingQueue[a]=incomingQueue[a];
  }
  outgoDue=true;
}
