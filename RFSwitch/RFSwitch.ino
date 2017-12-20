#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);
  
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);

  // Optional set pulse length.
  mySwitch.setPulseLength(600);
  
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);
  
  
  // Optional set number of transmission repetitions.
  //mySwitch.setRepeatTransmit(3);
  
}

void loop() {

  /* Same switch as above, but using decimal code */
  digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);
  delay(1000);
  digitalWrite(13,HIGH);
  /*mySwitch.send(6249742, 24);*/
  mySwitch.sendTriState("FF11FF1F0010");
  delay(400);
  mySwitch.sendTriState("FF11FF1F0010");
  delay(400);
  mySwitch.sendTriState("FF11FF1F0010");
  delay(400);
  mySwitch.sendTriState("FF11FF1F0010");
  delay(400);
  mySwitch.sendTriState("FF11FF1F0010");
  digitalWrite(13,LOW);
  delay(3000);
  Serial.println();
  
  /* Same switch as above, but using binary code */
  /*
  mySwitch.send("100000101100100011001000");
  delay(1000);
  */
}
