#define StateIDLE 1
#define StateSENDWAITING 2
#define StateRECEIVEWAITING 4
#define StateSENDING 5
#define StateRECEIVING 6
unsigned char currentState=StateIDLE;
#define RX1PIN 3
#define TX1PIN 1
void setup(){
  pinMode(RX1PIN,INPUT_PULLUP);
  pinMode(1,OUTPUT);
  DDRB=0xff;
}
void loop(){
  currentState=StateIDLE;
  PORTB=currentState;
  if(digitalRead(RX1PIN)){
    currentState=StateRECEIVING;
    PORTB=currentState;
    Serial.begin(9600);
    Serial.write(0xaa);
    Serial.write(0xbb);
    Serial.write(0xcc);
    Serial.end();
    digitalWrite(TX1PIN, LOW);
  }
}