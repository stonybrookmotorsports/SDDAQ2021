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
//  msg[ctr] = {SOME READ FUNCTION};
//  senid[ctr] = {ARBITRARY SENSOR ID ASSIGNED BY YOU};
//  sentim();
//
//For the first line, set it equal to a reading function, such as analogRead().
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
//If you have 3 sensors, you should have nine lines total.
//Lastly, you can remove the code that is currently in that section.
//
//================END OF EDITING INSTRUCTIONS=============


//+++++++++++++++++++++++UPDATE THESE TWO ONLY+++++++++++++++++++++++++
const long siz = 1; //PUT THE NUMBER OF SENSORS YOU'LL BE READING HERE
const int speriod = 125; //PUT THE TIME BETWEEN EACH READ GROUP HERE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

void setup() {
  Serial.begin(115200);

  //+++++++++++++++++++PINMODE DECLARATIONS+++++++++++++++++++
  //pinMode(LED_BUILTIN, OUTPUT);

  //++++++++++++++++END OF PINMODE DECLARATIONS+++++++++++++++

  //Initialize the array
  for (ctr = 0; ctr < siz; ctr++) {
    tim[ctr] = 1;
    msg[ctr] = 1;
  }
  ctr = 0;
  //digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  //digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  //digitalWrite(LED_BUILTIN, LOW);
  while (1) {
    if (Serial.available() > 0) {
      if (Serial.read() == 's') {
        Serial.write('b');
        break;
      }
    }
  }
  //digitalWrite(LED_BUILTIN, HIGH);

  //========HANDSHAKE=========
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
  //=============DATA COLLECTION============
  //If the buffers are not full, clct should be true, data is read
  if(clct){

    //++++++++++++++++++++++++DATA READINGS+++++++++++++++++++++++++++
    msg[ctr] = ictr;
    senid[ctr] = 3;
    sentim();
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
  delay(speriod);
}


//This function is responsible for writing the data
void datwrt(){
   for (i = 0; i < siz; i++) {
      Serial.write(senid[i]);
      Serial.write(tim[i] % 256);
      Serial.write((tim[i] / 256));
      Serial.write((tim[i] / 256 / 256));
      Serial.write(msg[i] % 256);
      Serial.write((msg[i] / 256));
    }
}

//This function sets the time of the reading and increments the counter
void sentim(){
    lastim = micros();
    tim[ctr] = (lastim - ctim) / 1000;
    ictr++;
    ctr++;
}
