//local libraries inclusion in arduino is a mess: https://arduino.stackexchange.com/questions/8651/loading-local-libraries
#include "SendOnlySoftwareSerial.h"
#include "ReceiveOnlySoftwareSerial.h"

#define TX 0
#define RX 1
//system allows also to have children and parents behind a mux
//in avr parenrx need be same as rx but different name for easier code
unsigned char PARENRX[]={2,4};
unsigned char PARENTX[]={3,5};
#define PARENTS 2
unsigned char CHILDRX[]={6,8};
unsigned char CHILDTX[]={7,9};
#define CHILDS 2

/*

     |TX -------------------------------------> |  computer Serial  |
     |             /--------------------------- |                   |
     |            /
PARENRX=RX  <----/--\--request permiss--- |      |          1
     |               \---uart receive-----|parent|          3
     |                                    |      |
  me |PARENTX ---------grant permission-> |      |          2
     |
     |CHILDRX[n] <---grant permission-- |     |             2
     |                                  |     |
     |CHILDTX[n] --\--uart send-------> |child|             3
     |              \--req. permiss---> |     |             1





*/


#define SENDBUTTONPIN 13
#define waitingToSendPin 10
#define waitingToReceivePin 11
#define idlePin 12

#define stIdle 0
#define stWaitingToSend 1
#define stWaitingToReceive 2
#define stSending 3
#define stReceiving 4

#define ofSendButton 0
#define ofSendMessage 1
#define lastFlag 1

unsigned char flags [lastFlag];
unsigned char currentState=0;
unsigned char waitingChild=0;

unsigned char messageOut[3];
unsigned char messageIn[3];

#define BAUDRATE 9600

//this method of communicating many childs and paraents is not what i wanted originally,
//however a prototype that follows this method should be compatible with one that follows
//the method I originally wanted.



//childRX are gouing to be just for analog reads
//childtx are softwareserials
SendOnlySoftwareSerial SchildTX[CHILDS] = {
  SendOnlySoftwareSerial(CHILDTX[0]),
  SendOnlySoftwareSerial(CHILDTX[1])
};
//parent tx are going to be just for analog writes
//parentRx are softwareserials
ReceiveOnlySoftwareSerial SparenRX[CHILDS] = {
  ReceiveOnlySoftwareSerial(PARENRX[0]),
  ReceiveOnlySoftwareSerial(PARENRX[1])
};

void setup ()
{
  for(unsigned char a = 0; a<CHILDS; a++){
    SchildTX[a].begin(BAUDRATE);
  }
  //reception softwareserials are only activated when a reception flag is set
  //so it doesn't mess up with receive permission flags

  Serial.begin(BAUDRATE);
  pinMode(SENDBUTTONPIN,INPUT);
  pinMode(waitingToSendPin,OUTPUT);
  pinMode(waitingToReceivePin,OUTPUT);
  pinMode(idlePin,OUTPUT);

  pinMode(PARENTX[0],OUTPUT);
  pinMode(PARENTX[1],OUTPUT);
  pinMode(CHILDRX[0],INPUT);
  pinMode(CHILDRX[1],INPUT);

  Serial.println("Hi");
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
      digitalWrite(idlePin,HIGH);
      //only idle state can lead to other states.
      //if there is a message to be sent
      if(flags[ofSendMessage]){
        startWaitingToSendAll();
      }
      bool parentReceptionWaiting=false;
      //check if a parent is asking permission to send a message
      for(unsigned char a=0; a<PARENTS; a++){
        if(digitalRead(PARENRX[a])){
          startMessageReceive(a);
          parentReceptionWaiting=true;
        }
      }
      Serial.write('.');
      //if there is no parent trying to send, means we are receiving from computer
      if(!parentReceptionWaiting){
        unsigned char n=0;
        if(Serial.available()){
          delay(10);
        }
        while(Serial.available()){
          messageOut[n]=Serial.read();
          n++;
          n%=3;
        }
        Serial.print("stored: ");
        Serial.write(messageOut[0]);
        Serial.write(messageOut[1]);
        Serial.write(messageOut[2]);
        Serial.write('\n');
      }
      break;
    }
    case stWaitingToSend:{
      digitalWrite(idlePin,LOW);
      waitingPermission();
      break;
    }
    case stWaitingToReceive:
      digitalWrite(idlePin,LOW);
    break;
    case stSending:
      digitalWrite(idlePin,LOW);
    break;

  }
}

//---  ran by stateCheck


//messages can only be sent to children (and the hardware serial)
void startWaitingToSendAll(){
  currentState=stWaitingToSend;
  digitalWrite(waitingToSendPin,HIGH);
  for(unsigned char childn=0; childn<CHILDS; childn++){
    //pendant: instead of setting a flag and the pin, it will be more secure
    //to set the pin and then use the pin as the flag
    waitingChild=0x1<<childn;
    digitalWrite(CHILDTX[childn],HIGH);
  }
}
void startMessageReceive(unsigned char a){
  currentState=stReceiving;
  //grant permission
  digitalWrite(PARENTX[a],HIGH);
  digitalWrite(waitingToReceivePin,HIGH);
  SparenRX[a].begin(BAUDRATE);

  long waitStart=millis();
  bool timeoutError=false;

  while(!SparenRX[a].available()){
    if(millis()-waitStart>3000) timeoutError=true;
  }

  unsigned char n=0;
  while(SparenRX[a].available()){
    messageIn[n]=SparenRX[a].read();
    n++;
    n%=3;
  }
  //send to computer what was received, for monitoring purposes
  Serial.print("received: ");
  Serial.write(messageIn[0]);
  Serial.write(messageIn[1]);
  Serial.write(messageIn[2]);
  Serial.write('\n');

  SparenRX[a].end();

  digitalWrite(PARENTX[a],LOW);
  currentState=stIdle;
  digitalWrite(waitingToReceivePin,LOW);
}
void waitingPermission(){
  //question: does the digital read interfere with the normal serial operation?
  //if so, perhaps we can receive permission from a parent using a different pin,
  //by somehow tying the RX and the ISPERMISSIONGRANTED pins
  for(unsigned char childn=0; childn<CHILDS; childn++){
    //if the iterated child is waiting to send message
    if((waitingChild>>childn)&0x1)
      if(digitalRead(CHILDRX[childn])){

        currentState=stSending;

        //send message to the iterated child
        SchildTX[childn].write(messageOut[0]);
        SchildTX[childn].write(messageOut[1]);
        SchildTX[childn].write(messageOut[2]);

        //clear the flag that message is waiting for this child
        waitingChild&=~(0x1<<childn);
        digitalWrite(CHILDTX[childn],LOW);
      }
  }
  Serial.println(String(waitingChild,BIN));
  if(waitingChild==0x0){
    digitalWrite(waitingToSendPin,LOW);
    currentState=stIdle;
  }
}



