#define StateIDLE 1
#define StateTimeoutError 3
#define StateSENDWAITING 2
#define StateRECEIVEWAITING 4
#define StateSENDING 5
#define StateRECEIVING 6
#define POLITEBAUD 9600
#define TIMEOUT 250
int currentState=StateIDLE;
long sendWaitStart=0;
long receiveWaitStart=0;
bool timeoutError=false;
void comLoopSetup(){
  pinMode(RX1PIN,INPUT);
  pinMode(TX1PIN,OUTPUT);
  // digitalWrite(TX1PIN,LOW);
  DDRB=0xff;
}
void comLoop(){
  PORTB=currentState;
  if(timeoutError){
    PORTB=StateTimeoutError;
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
      Serial1.begin(POLITEBAUD);
      receiveWaitStart=millis();
      timeoutError=false;
      int bytesCount=0;
      while(!((bytesCount>=MSGLEN)||timeoutError)){
        //count and timeout
        if(millis()-receiveWaitStart>TIMEOUT) timeoutError=true;
        if(Serial1.available()){
          incomingQueue[bytesCount]=Serial1.read();
          bytesCount++;
        }
        // delayMicroseconds(100);
      }
      Serial1.end();
      digitalWrite(TX1PIN, LOW);
      onPoliteMessage();
      currentState=StateIDLE;
      break;
    }
    case StateSENDING:{
      Serial1.begin(POLITEBAUD);
      for(unsigned char a=0; a<MSGLEN; a++){
        Serial1.write(outgoingQueue[a]);
      }
      Serial1.end();
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
}
void sendMessage(){
  currentState=StateSENDWAITING;
  sendWaitStart=millis();
}
