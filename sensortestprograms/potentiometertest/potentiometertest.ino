int val = 0;
int siz = 50;
int runav[50];
int i = 0;
int ctr = 0;
long fullav = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for(i=0;i<siz;i++){
    runav[i] = 0;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  val = analogRead(15);
  //Serial.println(val);
  runav[ctr]=val;

  ctr++;
  if(ctr >= siz){
    ctr = 0;
    for(i=0;i<siz;i++){
      fullav = fullav + runav[i];
    }
    //Serial.println(val);
    Serial.println(fullav/siz);
    fullav=0;
  }
  delay(2);
}
