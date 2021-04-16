// Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI slave device
// Refer to the "Wire Master Reader" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

unsigned char fh = 0;
unsigned char sh = 0;
unsigned char rpmbuf[2];
int ctr = 1;
long tim = 0;
long newtim = 0;
boolean stat = true;
int rpm = 0;

void setup() {
  //Serial.begin(115200);
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  attachInterrupt(digitalPinToInterrupt(2), yeet, RISING);
  tim = micros();
  rpmbuf[0] = 0;
  rpmbuf[1] = 0;
}

void loop() {
  //ctr++;
  //delay(100);
  if ((stat == true) && (ctr % 2 == 0)) {
    newtim = micros();
    //Serial.println("success");
    //Serial.println(newtim - tim);
    rpm = 1000000 / (newtim - tim);
    //Serial.println(rpm);
    tim = newtim;
    stat = false;
  }
  if (micros()-newtim > 1000000){
    rpm = 0;
    //Serial.println(rpm);
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  //fh = rpm % 256;
  //sh = rpm / 256;
  //Wire.write(fh);
  //Wire.write(sh);
  rpmbuf[0] = rpm % 256;
  rpmbuf[1] = rpm / 256;
  Wire.write(rpmbuf, 2);
  //Serial.println(rpm);
}

void yeet() {
  ctr++;
  stat = true;
}
