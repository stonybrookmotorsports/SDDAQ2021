const long siz = 10;
const char trotor = 20;
char tcount = 0;
int ctr = 0;
unsigned long ctim = 0;
unsigned long lastim = 0;
boolean collect = true;
boolean wrt = false;
int tim[siz];
int msg[siz];


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  for (ctr = 0; ctr < siz; ctr++) {
    tim[ctr] = 1;
    msg[ctr] = 1;
  }

  ctr = 0;


  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  

  while (1) {
    if (Serial.available() > 0) {
      if (Serial.read() == 's') {
        Serial.write('b');
        break;
      }
    }
  }

  digitalWrite(LED_BUILTIN, HIGH);


  while (1) {

    if (Serial.available() > 0) {
      if (Serial.read() == 'm') {
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
          ctr = 0;
          ctim = micros();
          lastim = micros();
          collect = true;
          break;
        }
      }
    }
  }
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);

}
