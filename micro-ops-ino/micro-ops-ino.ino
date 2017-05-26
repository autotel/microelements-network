#include <SendOnlySoftwareSerial.h>

#define TX 0
#define RX 1
//in avr parenrx need be same as rx but different name for easier code
#define PARENTX 3
#define CHILDRX 2
#define CHILDTX 4


/*

     |   TX ----------------------------------> |  computer Serial  |
     |             /--------------------------- |                   |
     |            /
PARENRX=RX  <----/--\--request permiss--- |      |          1
     |               \---uart receive-----|parent|          3
     |                                    |      |
  me |   PARENTX ------grant permission-> |      |          2
     |
     |   CHILDRX <---grant permission-- |     |             2
     |                                  |     |
     |   CHILDTX --\--uart send-------> |child|             3
     |              \--req. permiss---> |     |             1





*/


#define SENDBUTTONPIN 5


#define stIdle 0
#define stWaitingToSend 1
#define stWaitingToReceive 2
#define stSending 3

#define ofSendButton 0
#define ofSendMessage 1
#define lastFlag 1

unsigned char flags [lastFlag];
unsigned char currentState=0;
SendOnlySoftwareSerial SchildTX (CHILDTX);

void setup ()
{
  SchildTX.begin(9600);
  pinMode(SENDBUTTONPIN,INPUT_PULLUP);
}

void loop(){

  checkButton();
  stateCheck();
}
//--- ran by loop

void checkButton(){
  //debounced button press
  if(digitalRead(SENDBUTTONPIN)){
    if(flags[ofSendButton]!=127)
      flags[ofSendButton]++;
  }else{
    flags[ofSendButton]==0;
  }

  if(flags[ofSendButton]>30){
    flags[ofSendMessage]=true;
    flags[ofSendButton]=127;
  }
}


void stateCheck(){
  switch (currentState){
    case stIdle:{
      //only idle state can lead to other states.
      //if there is a message to be sent
      if(flags[ofSendMessage]){
        startWaitingToSend();
      }
      //check if a parent is asking permission to send a message
      if(digitalRead(PARENRX)){
        startMessageReceive();
      }
    }
    break;
    case stWaitingToSend:{
      //if the child gave me permission to send my message
      if(digitalRead(CHILDRX)){
        sendMessage();
      }
    }
    break;
    case stWaitingToReceive:
    break;
    case stSending:
    break;

  }
}

//---  ran by stateCheck


//messages can only be sent to children (and the hardware serial)
void startWaitingToSend(){
  currentState=stWaitingToSend;
  digitalWrite(CHILDTX,HIGH);
}
void startMessageReceive(){
  currentState=stWaitingToReceive;
  //grant permission
  digitalWrite(PARENTX,HIGH);
  long waitStart=millis();
  bool timeoutError=false;
  while(!Serial.available){
    if(millis()-waitStart>3000) timeoutError=true;
  }
  digitalWrite(PARENTX,LOW);
  currentState=stIdle;
}
void waitPermission(){
  //question: does the digital read interfere with the normal serial operation?
  //if so, perhaps we can receive permission from a parent using a different pin,
  //by somehow tying the RX and the ISPERMISSIONGRANTED pins
  if(digitalRead(CHILDRX)){
    currentState=stSending;

    SchildTX.write(messageOut[0]);
    SchildTX.write(messageOut[1]);
    SchildTX.write(messageOut[2]);

    currentState=stIdle;

    digitalWrite(CHILDTX,LOW);
  }
}



