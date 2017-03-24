#include <TimerOne.h>
#include <avr/sleep.h>

#define verde_carros 0
#define amarelo_carros 1
#define vermelho_carros 2
#define verde_pedestres 3
#define vermelho_pedestres 4

#define pino_botao 2
#define pino_LDR A0

unsigned int tempo_piscar;
unsigned int tempo_espera;
unsigned int tempo_noite;

int conta_tempo_noite = 0;
volatile int estado = 0;
int leds[5];

void RSI_timer()
{
  tempo_piscar++;
  if((!(tempo_piscar%10))&&(estado==1))
    tempo_espera++; //Contador de período 1s
  if((!(tempo_piscar%10))&&(conta_tempo_noite))
    tempo_noite++; //Contador de período 1s
  
  if((analogRead(pino_LDR) > 300)&&(estado!=1))
  {
    conta_tempo_noite = 1;
    if(tempo_noite > 5)
    {
      estado = 2;
      conta_tempo_noite = 0;
      tempo_noite = 0;
    }
  }
  if((analogRead(pino_LDR) < 300)&&(estado!=1))
  {
    conta_tempo_noite = 1;
    if(tempo_noite > 5)
    {
      estado = 0;
      conta_tempo_noite = 0;
      tempo_noite = 0;
    }
  }

}   

void RSI_botao()
{
  if(estado==0)
    estado = 1;
  //avalia_estado();
}

void configura_sinal(int a, int b, int c, int d, int e)
{
  leds[verde_carros] = a;
  leds[amarelo_carros] = b;
  leds[vermelho_carros] = c;
  leds[verde_pedestres] = d;
  leds[vermelho_pedestres] = e;
  leitura_vetor();
}

void leitura_vetor(){
  for(int i=0; i<5; i++){
    if(leds[i]==0)
      digitalWrite(i+5,LOW);
    else if(leds[i]==1)
      digitalWrite(i+5,HIGH);
    else if(leds[i]==2){
      if(tempo_piscar%10 < 5)
        digitalWrite(i+5,HIGH);
      else
        digitalWrite(i+5,LOW);
    }
  }
}


void avalia_estado()
{
  if(estado==0)
    aberto_carros();
  else if(estado==1)
    abre_pedestres();
  else if(estado==2)
    pisca_noite();

}

void aberto_carros()
{
  configura_sinal(1,0,0,0,1);
}

void abre_pedestres()
{
  if(tempo_espera < 2)
    configura_sinal(1,0,0,0,1);
  else if(tempo_espera < 4)
     configura_sinal(0,1,0,0,1);
  else if(tempo_espera < 6)
    configura_sinal(0,0,1,1,0);
  else if(tempo_espera < 8)
    configura_sinal(0,0,1,0,2);
  else if(tempo_espera >= 10)
  {
    estado = 0;
    tempo_espera = 0;
  }
}

void pisca_noite()
{
  configura_sinal(0,2,0,0,2);
}

void setup() {
  Timer1.initialize(100000); // Interrupcao a cada 0,1s
  Timer1.attachInterrupt(RSI_timer); // Associa a interrupcao periodica a funcao RSI_timer
  pinMode(pino_botao,INPUT);
  pinMode(pino_LDR,INPUT);
  pinMode(verde_carros+5,OUTPUT);
  pinMode(amarelo_carros+5,OUTPUT);
  pinMode(vermelho_carros+5,OUTPUT);
  pinMode(verde_pedestres+5,OUTPUT);
  pinMode(vermelho_pedestres+5,OUTPUT);
  //attachInterrupt(digitalPinToInterrupt(2), RSI_botao, RISING);
  attachInterrupt(0, RSI_botao, RISING);
  //avalia_estado();
  Serial.begin(2400);
}

void loop() {
  avalia_estado();
  
  // Entrar em modo SLEEP enquanto nada acontece
  //set_sleep_mode(SLEEP_MODE_IDLE);
  //sleep_enable();
  //sleep_mode();
  //sleep_disable();
}

//LM234
//inputcompare
