//here I learned that I can set a TX pin as output, set it LOW
//and whenever i serial.start, it will go high until serial.end
//which is very convenient because I can communicate the listening flag
//to the waiting sender, just by serial.start'ing

void setup(){

  pinMode(0,OUTPUT);
  digitalWrite(0,LOW);

}
void loop(){
  delay(599);
  Serial.begin(9600);
  delay(566);
  Serial.end();
}