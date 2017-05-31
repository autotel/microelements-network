/*
  A library to make serial communications polite;
  meaning that to send a message, a flag will be set,
  and the message will not be sent until a permission
  flag is set by the reciever.

  Created by Joaquín Aldunate  http://autotel.co
  //license?
*/
#include "Arduino.h"
#ifndef PoliteSerial_h
#define PoliteSerial_h
#define MSGLEN 4
class PoliteSerial{
  public:
    PoliteSerial(HardwareSerial &ms,int myRX, int myTX, int baudRate);
    void init();
    int loop();
    void onMessage(void (*_midiInCallback)());
    void sendMessage();
    unsigned char outgoingQueue [MSGLEN];
    unsigned char incomingQueue [MSGLEN];
    
  private:
    HardwareSerial& _Serial;
    int currentState;
    long sendWaitStart=0;
    long receiveWaitStart=0;
    bool timeoutError=false;
    int POLITEBAUD = 9600;
    int TIMEOUT = 250;
    unsigned char RX1PIN;
    unsigned char TX1PIN;
    void (*_midiInCallback)();
};

#endif
