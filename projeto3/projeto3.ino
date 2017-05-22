#include "TimerOne.h"

#define pinoSensor 2
#define pinoMotor 3

const float alpha = 0.001;
long int contador = 0;
float vel_atual = 0;
float vel_desejada = 3000;
float vel_max = 6000;
uint8_t pwm = 127;

void RSI_sensor(){
  contador++; //Incrementa o contador cada vez que a pa passa na frente do sensor
}

void RSI_timer(){
  vel_atual = (contador / 0.05) * 30; //Calcula velocidade em RPM
  contador = 0; //Zera o contador
}

void setup() {
  //Configura portas como entrada/saida
  pinMode(pinoMotor, OUTPUT);
  pinMode(pinoSensor, INPUT);
  
  //Configura interrupcao por sinal
  attachInterrupt(0, RSI_sensor, RISING);
  //Configura interrupcao periodica com T=0.05s
  Timer1.initialize(50000);
  Timer1.attachInterrupt(RSI_timer);
  
  Serial.begin(9600);
}

void loop() {
  float erro = vel_atual - vel_desejada;
  int8_t ajuste = (int8_t)(alpha*erro);
  if((int16_t) (pwm - ajuste) > 255)
    pwm = 255;
  else if((int16_t) (pwm - ajuste) < 0)
    pwm = 0;
  else
    pwm = pwm - ajuste;
    
  analogWrite(pinoMotor,pwm);
  Serial.println(vel_atual);
}
