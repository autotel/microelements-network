#include "Arduino.h"
#include "PoliteSerial.h"

#define StateIDLE 1
#define StateTimeoutError 3
#define StateSENDWAITING 2
#define StateRECEIVEWAITING 4
#define StateSENDING 5
#define StateRECEIVING 6

PoliteSerial::PoliteSerial(HardwareSerial *ms, int myRX, int myTX, int baudRate) {
  POLITEBAUD = baudRate;
  _Serial = ms;
  RX1PIN = myRX;
  TX1PIN = myTX;
  pinMode(RX1PIN, INPUT);
  pinMode(TX1PIN, OUTPUT);
  currentState = StateIDLE;
  // DDRB=0xff;
}
PoliteSerial::PoliteSerial(SoftwareSerial *ms, int myRX, int myTX, int baudRate) {
  POLITEBAUD = baudRate;
  _SSerial = ms;
  RX1PIN = myRX;
  TX1PIN = myTX;
  pinMode(RX1PIN, INPUT);
  pinMode(TX1PIN, OUTPUT);
  currentState = StateIDLE;
  // DDRB=0xff;
  softSerialMode = true;
}
void PoliteSerial::init() {

}
void PoliteSerial::onMessage(void (*midiInCallback)())
{
  _midiInCallback = midiInCallback;
}
int PoliteSerial::loop() {
  // PORTB=currentState;
  if (timeoutError) {
    // PORTB=StateTimeoutError;
    delay(TIMEOUT * 3);
    timeoutError = false;
  }
  // delay(100);
  switch (currentState) {
    case StateIDLE: {
        digitalWrite(TX1PIN, LOW);
        if (digitalRead(RX1PIN)) {
          currentState = StateRECEIVING;
        }
        break;
      }
    case StateRECEIVING: {
        digitalWrite(TX1PIN, HIGH);//comment this
        //begins the serial, that will set the pin to high in consecuence,
        //giving a permit flag
        
        mySerialBegin();
        receiveWaitStart = millis();
        timeoutError = false;
        int bytesCount = 0;
        while (!((bytesCount >= MSGLEN) || timeoutError)) {
          //count and timeout
          if (millis() - receiveWaitStart > TIMEOUT) timeoutError = true;
          if (mySerialAvailable()) {
            incomingQueue[bytesCount] = mySerialRead();
            bytesCount++;
          }
          // delayMicroseconds(100);
        }
        mySerialEnd();
        digitalWrite(TX1PIN, LOW);
        // onPoliteMessage();
        _midiInCallback();
        currentState = StateIDLE;
        break;
      }
    case StateSENDING: {
        mySerialBegin();
        for (unsigned char a = 0; a < MSGLEN; a++) {
          mySerialWrite(outgoingQueue[a]);
        }
        mySerialEnd();
        digitalWrite(TX1PIN, LOW);
        currentState = StateIDLE;
        break;
      }
    case StateSENDWAITING: {
        timeoutError = (millis() - sendWaitStart) > TIMEOUT;
        if (timeoutError) currentState = StateIDLE;
        digitalWrite(TX1PIN, HIGH);
        if (digitalRead(RX1PIN)) {
          currentState = StateSENDING;
        }
        break;
      }
  }
  return currentState;
}

//pendant: the Serial Selection should be a preprocessor directive, not at runtime. But I don't know how to do that
void PoliteSerial::mySerialBegin() {
  if (softSerialMode) {
    _Serial->begin(POLITEBAUD);
  } else {
    _SSerial->begin(POLITEBAUD);
  }
}
void PoliteSerial::mySerialEnd() {
  if (softSerialMode) {
    _Serial->end();
  } else {
    _SSerial->end();
  }
}
void PoliteSerial::mySerialWrite(unsigned char data) {
  if (softSerialMode) {
    _Serial->write(data);
  } else {
    _SSerial->write(data);
  }
}
unsigned char PoliteSerial::mySerialRead() {
  if (softSerialMode) {
    return _Serial->read();
  } else {
    return _SSerial->read();
  }
}
bool PoliteSerial::mySerialAvailable() {
  if (softSerialMode) {
    return _Serial->available();
  } else {
    return _SSerial->available();
  }
}

void PoliteSerial::sendMessage() {
  currentState = StateSENDWAITING;
  sendWaitStart = millis();
}
