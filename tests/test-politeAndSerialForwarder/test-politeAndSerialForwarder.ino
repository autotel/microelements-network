//arduino Mega
#define RCBLEN 16
#define MSGLEN 4
char rcb [RCBLEN];
unsigned char wrHead=0;
unsigned char readHead=0;
#define RX1PIN 19
#define TX1PIN 18
bool outgoDue=false;

unsigned char outgoingQueue [MSGLEN];
unsigned char incomingQueue[MSGLEN];

void setup(){
  Serial.begin(9600);
  Serial.println("hi");
  comLoopSetup();
}

void loop(){
  if(Serial.available()){
    Serial.println("received@"+String(millis(),DEC));
    delay(10);
    while(Serial.available()){
      rcb[wrHead]=Serial.read();
      wrHead++;
      wrHead%=RCBLEN;
    }
  }

  unsigned char outgoheader=0;

  while((readHead)!=wrHead){
    // Serial1.write(rcb[readHead]);
    Serial.write(rcb[readHead]);
    outgoingQueue[outgoheader]=rcb[readHead];
    outgoheader++;
    readHead++;
    readHead%=RCBLEN;
    outgoDue=true;
  }
  comLoop();
  if(outgoDue){
    Serial.println("sending@"+String(millis(),DEC));
    sendMessage();
    outgoDue=false;
  }
}

void onVirtualMessage(){
  for(unsigned char a=0; a<MSGLEN;a++)
  Serial.write(incomingQueue[a]);
  Serial.println("polite received@"+String(millis(),DEC));
}
