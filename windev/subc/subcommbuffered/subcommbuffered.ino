const long siz = 1;
const int speriod = 250;
const char trotor = 20;
char tcount = 0;
unsigned long ctim = 0;
unsigned long lastim = 0;
byte senid[siz];
unsigned long tim[siz];
int msg[siz];
boolean clct = true;
boolean wrt = false;
char ctr = 0;
int ictr = 0;
char i = 0;

void setup() {
  Serial.begin(115200);
  //pinMode(LED_BUILTIN, OUTPUT);

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

  ctim = micros();
  lastim = micros();
}

void loop() {
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

  //collecting data
  if(clct){
    lastim = micros();
    tim[ctr] = (lastim - ctim) / 1000;
    //Serial.println("clct");
    //Serial.println(tim[ctr]);
    msg[ctr] = ictr;
    senid[ctr] = 7;
    //Serial.println(msg[ctr]);
    ictr++;
    ctr++;
    if(ctr >= siz){
      clct = false;
      wrt = true;
      ctr=0;
    }
  }
  if(wrt){
    datwrt();
    ctr = 0;
    wrt = false;
    clct = true;
  }
  delay(speriod);
}

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