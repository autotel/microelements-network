#define RCBLEN 16
char rcb [RCBLEN];
unsigned char wrHead=0;
unsigned char readHead=0;
#define RXPIN 0
#define TXPIN 1
#define VARPULLDOWNPIN 2
void setup(){
  pinMode(RXPIN,INPUT);
  pinMode(TXPIN,OUTPUT);
  pinMode(VARPULLDOWNPIN,OUTPUT);
  digitalWrite(VARPULLDOWNPIN,LOW);
  digitalWrite(TXPIN,LOW);
}
void loop(){
  //you can digitalRead the pin, but it will detect serial signals as lows
  if(digitalRead(RXPIN)){
    Serial.begin(9600);
    digitalWrite(VARPULLDOWNPIN,HIGH);
    Serial.println("Serial activate");
    if(Serial.available()){
      delay(10);
      while(Serial.available()){
        rcb[wrHead]=Serial.read();
        wrHead++;
        wrHead%=RCBLEN;
      }
    }
    digitalWrite(VARPULLDOWNPIN,LOW);
    Serial.end();
    digitalWrite(TXPIN,LOW);
  }

  while((readHead)!=wrHead){
    Serial.begin(9600);
    digitalWrite(VARPULLDOWNPIN,HIGH);
    Serial.println("send");
    Serial.write(rcb[readHead]);
    readHead++;
    readHead%=RCBLEN;
    digitalWrite(VARPULLDOWNPIN,LOW);
    Serial.end();
    digitalWrite(TXPIN,LOW);
  }
}