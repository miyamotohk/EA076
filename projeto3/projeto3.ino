#define pinoMotor 3
#define pinoSensor 4

void setup() {
  pinMode(pinoMotor, OUTPUT);
  pinMode(pinoSensor, INPUT);
  Serial.begin(9600);
}

void loop() {
  //teste dos motores
  /*for(int i=0; i<255; i++){
    analogWrite(pinoMotor,i);
    delay(10);
  }
  for(int i=255; i>0; i--){
    analogWrite(pinoMotor,i);
    delay(10);
  }*/
  if(digitalRead(pinoSensor))
    Serial.println("HIGH");
  else
    Serial.println("LOW");
}
