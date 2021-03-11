//===================EDITING INSTRUCTIONS=================
//
//Very little should be changed.
//Only update things between the comments comments that are lines of +'s.
//
//
//Update the first two global variables depending on number of sensors/delay.
//The speriods for all subcontrollers should be divisible by the one with the lowest delay.
//For example, if the lowest delay for a SC is 125, the others can have delays of 250, 375, 500, etc.
//
//
//In setup(), make any necessary PINMODE() calls in the designated area.
//
//
//For the editing loop:
//Each sensor reading must use the following three lines:
//
//  msg[ctr] = {SOME READ FUNCTION OR VALUE};
//  senid[ctr] = {ARBITRARY SENSOR ID ASSIGNED BY YOU};
//  sentim();
//
//For the first line, set it equal to a reading function, such as analogRead(), or a value.
//For the second line, set a unique and arbitrary sensor ID for each sensor, note it down for testing.
//For the third line, do not change anything, and make sure to always call it.
//For a given reading, the three lines MUST be in order. The sentim() call can't come before.
//The time, sensor id, and message arrays will all have slots for each sensor if you set siz correctly.
//An example is shown below:
//
//  msg[ctr] = analogRead(A3);
//  senid[ctr] = 3;
//  sentim();
//
//Make sure you have these three lines repeated for each sensor.
//If you have 3 sensors, you should have nine lines total at least
//If you need, you can add additional code required to obtain values
//Note that whatever you do, it is important those three lines exist.
//Lastly, you can remove the code that is currently in that section.
//
//================END OF EDITING INSTRUCTIONS=============

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


//+++++++++++++++++++++++UPDATE THESE TWO ONLY+++++++++++++++++++++++++
const long siz = 23; //PUT THE NUMBER OF SENSORS YOU'LL BE READING HERE
const int speriod = 50; //PUT THE TIME BETWEEN EACH READ GROUP HERE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//========IMU VARIABLES========
//uint16_t imudelay = speriod;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);


//=============================

//========VARIABLE DECLARATION==========
unsigned long ctim = 0; //Start time variable
unsigned long lastim = 0; //Last time variable
byte senid[siz]; //Buffer for sensor IDs
unsigned long tim[siz]; //Buffer for time values
int msg[siz]; //Buffer for sensor data
boolean clct = true; //Boolean for collect data
boolean wrt = false; //Boolean for writing data
char ctr = 0; //Counter for number of sensor of which have been read from
int ictr = 0; //Counter for total number of reads
char i = 0; //Iterator
unsigned char buf[2];
byte wirectr = 0;
//byte LEDP = 4;
byte poten = 2;

void setup() {
  Serial.begin(115200);
  bno.begin();
  //+++++++++++++++++++PINMODE DECLARATIONS+++++++++++++++++++
  //pinMode(LED_BUILTIN, OUTPUT);
  //Wire.begin();        // join i2c bus (address optional for master)  
  buf[0] = 0;
  buf[1] = 0;
  //pinMode(LEDP, OUTPUT);
  //++++++++++++++++END OF PINMODE DECLARATIONS+++++++++++++++

  //Initialize the array
  for (ctr = 0; ctr < siz; ctr++) {
    tim[ctr] = 1;
    msg[ctr] = 1;
  }
  ctr = 0;
  //digitalWrite(LEDP, LOW);
  //delay(500);
  //digitalWrite(LEDP, HIGH);
  //delay(500);
  //digitalWrite(LEDP, LOW);
  while (1) {
    if (Serial.available() > 0) {
      if (Serial.read() == 's') {
        Serial.write('b');
        break;
      }
    }
  }
  //digitalWrite(LEDP, HIGH);

  //========HANDSHAKE=========`
  while (1) {
    if (Serial.available() > 0) {
      if (Serial.read() == 'm') {
        Serial.write(speriod % 256);
        Serial.write(speriod / 256);
        Serial.write(siz);
        break;
      }
    }
  }
  //Set the time variables to start
  ctim = micros();
  lastim = micros();
}

void loop() {
  //New handshake incase connection to main controller is lost

  /*
  if (Serial.available() > 0) {
    if (Serial.read() == 's') {
      Serial.write('b');
    }
    while (1) {
      if (Serial.available() > 0) {
        if (Serial.read() == 'm') {
          Serial.write(speriod % 256);
          Serial.write(speriod / 256);
          Serial.write(siz);
          ictr = 0;
          ctr = 0;
          ctim = micros();
          lastim = micros();
          clct = true;
          wrt = false;
          break;
        }
      }
    }
  }
  
  */
  //=============DATA COLLECTION============
  //If the buffers are not full, clct should be true, data is read
  //digitalWrite(LEDP, LOW);
  if(clct){

    //++++++++++++++++++++++++DATA READINGS+++++++++++++++++++++++++++
    //Serial.println("Here");

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //From here until the next tilde (~) comment line, the code is adapted from
    //the adafruit "read_all_data" example file in the BN0O55 example library. 
    //The specific lines are 56-69, 71
    sensors_event_t orientationData , angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
    bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
    bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
    bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
    bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);
  
    writeEvent(&orientationData, 1);
    writeEvent(&angVelocityData, 4);
    writeEvent(&linearAccelData, 7);
    writeEvent(&magnetometerData, 10);
    writeEvent(&accelerometerData, 13);
    writeEvent(&gravityData, 16);
  
    int8_t boardTemp = bno.getTemp();
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    
    msg[ctr] = boardTemp;
    senid[ctr] = 19;
    sentim();

  
    uint8_t system, gyro, accel, mag = 0; //This is from line 76 of the example
    bno.getCalibration(&system, &gyro, &accel, &mag); //This is from line 77 of the example
    //Serial.println();
    msg[ctr] = system;
    senid[ctr] = 20;
    sentim();
    msg[ctr] = gyro;
    senid[ctr] = 21;
    sentim();
    msg[ctr] = accel;
    senid[ctr] = 22;
    sentim();
    msg[ctr] = mag;
    senid[ctr] = 23;
    sentim();

    /*
    Serial.print("Calibration: Sys=");
    Serial.print(system);
    Serial.print(" Gyro=");
    Serial.print(gyro);
    Serial.print(" Accel=");
    Serial.print(accel);
    Serial.print(" Mag=");
    Serial.println(mag);
  
    Serial.println("--");
    */
        
    //+++++++++++++++++++++++END DATA READING+++++++++++++++++++++++++

    //If the buffer is full, write the data
    if(ctr >= siz){
      clct = false;
      wrt = true;
      ctr=0;
    }
  }

  //If the buffers are full, write the data
  if(wrt){
    datwrt();
    ctr = 0;
    wrt = false;
    clct = true;
  }

  //delay(speriod/2);
  //digitalWrite(LEDP, HIGH);
  //delay(speriod/2);
  delay(speriod-10);
}


//This function is responsible for writing the data
void datwrt(){
   //digitalWrite(LEDP, HIGH);
         //Serial.println("writing");
   for (i = 0; i < siz; i++) {
      
      Serial.write(senid[i]);
      Serial.write(tim[i] % 256);
      Serial.write((tim[i] / 256));
      Serial.write((tim[i] / 256 / 256));
      Serial.write(msg[i] % 256);
      Serial.write((msg[i] / 256));
      /**/
      /*=====These commented out sections are here for debugging======
      Serial.print("ictr: ");
      Serial.println(ictr);
      Serial.println(senid[i]);
      Serial.println(tim[i]);
      Serial.println(msg[i]);

      /*
      Serial.println((unsigned char)(senid[i]));
      Serial.println((unsigned char)(tim[i] % 256));
      Serial.println((unsigned char)(tim[i] / 256));
      Serial.println((unsigned char)(tim[i] / 256 / 256));
      Serial.println((unsigned char)(msg[i] % 256));
      Serial.println((unsigned char)(msg[i] / 256));
      /*
      */
  }
     //digitalWrite(LEDP, LOW);
}

//This function sets the time of the reading and increments the counter
void sentim(){
    lastim = micros();
    tim[ctr] = (lastim - ctim) / 1000;
    ictr++;
    ctr++;
}

//THIS FUNCTION IS PROVIDED BY THE ADAFRUIT BNO055 EXAMPLES AND WAS WRITTEN BY THEM
//THIS FUNCTION SPECFICALLY COMES FROM "read_all_data.ino", LINES 92-132

void writeEvent(sensors_event_t* event, byte senidn) {
  double x = -1000000, y = -1000000 , z = -1000000; //dumb values, easy to spot problem
  if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    //Serial.print("Accl:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_ORIENTATION) {
    //Serial.print("Orient:");
    x = event->orientation.z;
    y = event->orientation.y;
    z = event->orientation.x;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    //Serial.print("Mag:");
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if (event->type == SENSOR_TYPE_GYROSCOPE) {
    //Serial.print("Gyro:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_ROTATION_VECTOR) {
    //Serial.print("Rot:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION) {
    //Serial.print("Linear:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else {
    //Serial.print("Unk:");
  }

  msg[ctr] = (int)(x*100);
  senid[ctr] = senidn;
  sentim();
  msg[ctr] = (int)(y*100);
  senid[ctr] = senidn+1;
  sentim();
  msg[ctr] = (int)(z*100);
  senid[ctr] = senidn+2;
  sentim();
}
