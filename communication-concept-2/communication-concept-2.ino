//ascii schematic at the bottom
#define PERIPHERALS 4;

#define PRXPIN 3
#define PTXPIN 4

#define MSGLEN 3

#define MODE_ECHO 0
#define MODE_FORWARD 1

#define SERIALWAITPAUSE 10

unsigned char myMode=MODE_FORWARD;

bool messageSendIsDue=false;

unsigned char incomingMessage [MSGLEN];
unsigned char outgoingMessage [MSGLEN];

void setup(){
  if(myMode==MODE_FORWARD){
    ComputerSerial.begin(9600);
  }
  pinMode(PRXPIN,INPUT);
}
void loop(){
  checkIncoming();
  outGo();
  if(myMode==MODE_FORWARD){
    unsigned char b=0;
    if(ComputerSerial.available()){
      delay(SERIALWAITPAUSE);

      while (ComputerSerial.available()) {
        messageSendIsDue=true;
        outgoingMessage[b]=ComputerSerial.read();
        b++;
        b%=MSGLEN;
        delay(SERIALWAITPAUSE);
      }
    }

  }
}
void checkIncoming(){
  for(unsigned char a=0; a<PERIPHERALS; a++){
    writeMuXaddresss(a);
    //question no°1: can one use the same pin to read and to receive serial?
    //will another arduino be able to pull up this pin, and would this arduino read it?
    if(digitalRead(PRXPIN)){
      //grant permission
      digitalWrite(PTXPIN,HIGH);
      //wait and receive serial
      PeripheralSerial.begin(9600);
      while(!PeripheralSerial.available()){
        //count time to generate a timeout error if is the case
      }
      delay(SERIALWAITPAUSE);
      unsigned char b=0;
      while(PeripheralSerial.available()){
        incomingMessage[b]=PeripheralSerial.read();
        if(myMode==MODE_ECHO){
          outgoingMessage=incomingMessage[b];
          messageSendIsDue=true;
        }
        b++;
        b%=MSGLEN;
      }
      PeripheralSerial.end();
    }
  }
}
//question n°2: here we will need some sort of children logging or pullup solution,
//because if there is no children in one of the mux addresses, this arduino will have
//to wait until a timeout is generated.
//it may aswell cancel the message in a short timeout and just retry on the next loop cycle
//if there is no child, it will keep skipping. if there is a child but busy, it will also skip
//but the next loop should come soon! it makes a good replacement of waiting until ready

//question n°3 the UART runs in parallel, I though this code as if UART ran sequentially
//what changes does it need to work? maybe a global state? I think that my code blocks the
//whole arduino until message is fully received.

void outGo(){
  if(!messageSendIsDue)
    return;
  //ask permission to send
  digitalWrite(PTXPIN,HIGH);
  for(unsigned char a=0; a<PERIPHERALS; a++){
    writeMuXaddresss(a);
    //wait a time for the recepient to grant permission.
    //if permission not granted on time, it will retry on next cycle of the loop
    delay(10);
    if(digitalRead(PRXPIN)){
      PeripheralSerial.begin(9600);
      for(unsigned char b=0; b<MSGLEN; b++){
        PeripheralSerial.write(outgoingMessage[b]);
      }
      PeripheralSerial.end();
    }
  }
}





/*


          |              | M |-------conn1tx-------------°
          |--------------| U |-------conn2tx  conn1rx----°
          |--------------| X |-------conn3tx
          |     tx-------| T |-------conn4tx  conn2tx----°
me,       |                                   conn2rx----°
arduino   |              | M |-------conn1rx
muxaddr<<0|--------------| U |-------conn2rx  conn3tx----°
muxaddr<<1|--------------| X |-------conn3rx  conn3rx----°
        RX|--------------| R |-------conn4rx
        TX|-----tx                            conn4tx----°
          |                                   conn4rx----°



*/