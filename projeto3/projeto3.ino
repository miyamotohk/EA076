#include "TimerOne.h"

#define pinoSensor 2
#define pinoMotor 3

//Variaveis globais para controle de processos
const float alpha = 0.0007;   //Parametro do controle proporcional (obtido empiricamente)
long int contador = 0;       //Contador de passagens do obstaculo pelo sensor
float vel_atual = 0;         //Velocidade atual do motor (calculada em RSI_sensor)
float vel_desejada = 3000;   //Velocidade desejada para o motor (deve ser ajustada manualmente no codigo)
float vel_max = 6000;        //Velocidade maxima do motor (estimada a partir das suas especificacoes)
uint8_t pwm = 127;           //Valor enviado para o motor na primeira iteracao do controle

//RSI acionada pela detecao de um obstaculo pelo sensor
void RSI_sensor(){
  contador++; //Incrementa o contador cada vez que a pa passa na frente do sensor
}

//RSI periodica para calculo da velocidade
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
  
  //Inicializa comuincacao serial
  Serial.begin(9600);
}

void loop() {
  //Rotina de controle proporcional do motor
  float erro = vel_atual - vel_desejada; //Calcula erro
  int8_t ajuste = (int8_t)(alpha*erro); //Calcula ajuste
  
  //Calucla valor valor ajustado (pwm - ajuste), que deve estar entre 0 e 255
  if((int16_t) (pwm - ajuste) > 255)
    pwm = 255; //Se for maior que 255, satura em 255
  else if((int16_t) (pwm - ajuste) < 0)
    pwm = 0; //Se for menor que 0, satura em 0
  else
    pwm = pwm - ajuste;
  
  analogWrite(pinoMotor,pwm); //Envia valor pwm para o motor
  Serial.println(vel_atual); //Escreve velocidade atual no monitor serial para acompanhamento
}
