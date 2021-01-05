char senidtostor = 0;
unsigned int timtostor = 0;
unsigned int msgtostor = 0;
unsigned long ctim = 0;
unsigned long lastim = 0;
unsigned long tim1 = 0;
unsigned long tim2 = 0;
unsigned long tim3 = 0;
unsigned long tim4 = 0;
unsigned long tim5 = 0;
unsigned long tim6 = 0;
const int bsiz = 100;
byte buf[bsiz];

void setup() {
  Serial.begin(115200);
  pinMode(5, INPUT);
  for(int j = 0; j<bsiz; j++){
    buf[j] = digitalRead(5);
    buf[j] = j;
  }
  delay(5000);
  ctim = lastim = micros();

}

void loop() {
  ctim = micros();
  lastim=ctim;

  for(int i=0;i<bsiz;i++){
    buf[i] = digitalRead(5);
    buf[i] = i;
    Serial.write(buf[i]);
  }


  ctim = micros();
  tim1 = ctim-lastim;
  lastim=ctim;

  for(int j = 0; j<bsiz; j++){
    buf[j] = digitalRead(5);
    buf[j] = buf[j] * 3 / 4 + 5;
  }



  ctim = micros();
  tim2 = ctim-lastim;
  lastim=ctim;

  Serial.write(buf, bsiz);

  
  ctim = micros();
  tim3 = ctim-lastim;
  lastim=ctim;


  for(int i=0;i<bsiz;i++){
    buf[i] = digitalRead(5);
    buf[i] = buf[i] * 3 / 4 + 5;
    Serial.write(buf[i]);
  }


  ctim = micros();
  tim4 = ctim-lastim;
  lastim=ctim;

  for(int j = 0; j<bsiz; j++){
    buf[j] = digitalRead(5);
    buf[j] = buf[j] * 3 / 4 + 5;
  }



  ctim = micros();
  tim5 = ctim-lastim;
  lastim=ctim;

  for(int k=0;k<bsiz;k++){
    Serial.write(buf[k]);
  }

  
  ctim = micros();
  tim6 = ctim-lastim;

  Serial.println("e");
  Serial.println(tim1);
  Serial.println("e");
  Serial.println(tim2);
  Serial.println("e");
  Serial.println(tim3);
  Serial.println("e");
  Serial.println(tim4);
  Serial.println("e");
  Serial.println(tim5);
  Serial.println("e");
  Serial.println(tim6);
  Serial.flush();
  delay(100);
}
