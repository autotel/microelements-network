//how to make a circular buffer, Joaquín Aldunate
#define RCBLEN 16
char rcb [RCBLEN];
unsigned char wrHead=0;
unsigned char readHead=0;
void setup(){
  Serial.begin(9600);
  Serial.print("hi");
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
  while((readHead)!=wrHead){
    Serial.write(rcb[readHead]);
    readHead++;
    readHead%=RCBLEN;
  }
}