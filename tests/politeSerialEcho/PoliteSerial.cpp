#include "Arduino.h"
#include "PoliteSerial.h"

#define StateIDLE 1
#define StateTimeoutError 3
#define StateSENDWAITING 2
#define StateRECEIVEWAITING 4
#define StateSENDING 5
#define StateRECEIVING 6

PoliteSerial::PoliteSerial(HardwareSerial& ms,int myRX, int myTX, int baudRate):
_Serial(ms){
  POLITEBAUD=baudRate;
  //_Serial=ms;
  RX1PIN=myRX;
  TX1PIN=myTX;
  pinMode(RX1PIN,INPUT);
  pinMode(TX1PIN,OUTPUT);
  currentState=StateIDLE;
  // DDRB=0xff;
}
void PoliteSerial::init(){
  
}
void PoliteSerial::onMessage(void (*midiInCallback)())
{
	_midiInCallback = midiInCallback;
}
int PoliteSerial::loop(){
  // PORTB=currentState;
  if(timeoutError){
    // PORTB=StateTimeoutError;
    delay(TIMEOUT*3);
    timeoutError=false;
  }
  // delay(100);
  switch (currentState){
    case StateIDLE:{
      digitalWrite(TX1PIN,LOW);
      if(digitalRead(RX1PIN)){
        currentState=StateRECEIVING;
      }
      break;
    }
    case StateRECEIVING:{
      digitalWrite(TX1PIN, HIGH);//comment this
      //begins the serial, that will set the pin to high in consecuence,
      //giving a permit flag
      _Serial.begin(POLITEBAUD);
      receiveWaitStart=millis();
      timeoutError=false;
      int bytesCount=0;
      while(!((bytesCount>=MSGLEN)||timeoutError)){
        //count and timeout
        if(millis()-receiveWaitStart>TIMEOUT) timeoutError=true;
        if(_Serial.available()){
          incomingQueue[bytesCount]=_Serial.read();
          bytesCount++;
        }
        // delayMicroseconds(100);
      }
      _Serial.end();
      digitalWrite(TX1PIN, LOW);
      // onPoliteMessage();
      _midiInCallback();
      currentState=StateIDLE;
      break;
    }
    case StateSENDING:{
      _Serial.begin(POLITEBAUD);
      for(unsigned char a=0; a<MSGLEN; a++){
        _Serial.write(outgoingQueue[a]);
      }
      _Serial.end();
      digitalWrite(TX1PIN, LOW);
      currentState=StateIDLE;
      break;
    }
    case StateSENDWAITING:{
      timeoutError=(millis()-sendWaitStart)>TIMEOUT;
      if(timeoutError) currentState=StateIDLE;
      digitalWrite(TX1PIN, HIGH);
      if(digitalRead(RX1PIN)){
        currentState=StateSENDING;
      }
      break;
    }
  }
  return currentState;
}

void PoliteSerial::sendMessage(){
  currentState=StateSENDWAITING;
  sendWaitStart=millis();
}
