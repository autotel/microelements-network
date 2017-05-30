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

class PoliteSerial
{
  public:
    PoliteSerial();
    void init(Stream &ms,int myRX, int myTX, int baudRate);
    void loop();
    void onMessage(void (*midiInCallback)(MidiMessage));
    unsigned char outgoingQueue [MSGLEN];
    unsigned char incomingQueue [MSGLEN];
  private:
    Stream _Serial
    int currentState;
    long sendWaitStart=0;
    long receiveWaitStart=0;
    bool timeoutError=false;
    int POLITEBAUD = 9600;
    int TIMEOUT = 250;
    unsigned char RX1PIN;
    unsigned char TX1PIN;
};

#endif
