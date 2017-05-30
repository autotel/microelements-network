#define RCBLEN 16
char rcb [RCBLEN];
unsigned char wrHead=0;
unsigned char readHead=0;
#define RXPIN 0
void setup(){
  Serial.begin(9600);
  Serial.print("hi");
  pinMode(RXPIN,INPUT_PULLUP);
}
void loop(){
  if(Serial.available()){
    delay(10);
    while(Serial.available()){
      rcb[wrHead]=Serial.read();
      wrHead++;
      wrHead%=RCBLEN;
    }
  }

  //you can digitalRead the pin, but it will detect serial signals as lows
  if(digitalRead(RXPIN)==LOW)
  while((readHead)!=wrHead){
    Serial.write(rcb[readHead]);
    readHead++;
    readHead%=RCBLEN;
  }
}