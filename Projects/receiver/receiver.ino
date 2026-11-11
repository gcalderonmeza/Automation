#include <RCSwitch.h> 

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
