#include <TimerOne.h>
#include <avr/sleep.h>

#define verde_carros 0
#define amarelo_carros 1
#define vermelho_carros 2
#define verde_pedestres 3
#define vermelho_pedestres 4

#define pino_botao 2

unsigned int tempo_piscar;
unsigned int tempo_espera;
volatile int estado = 0;
int leds[5];

void RSI_timer()
{
  tempo_piscar++;
  if(!(tempo_piscar%10))
    tempo_espera++; //Contador de período 1s
}   

void RSI_botao()
{
  //abre_pedestres();
  estado = 1;
  avalia_estado();
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
    pisca_noite;
}

void aberto_carros()
{
  configura_sinal(1,0,0,0,1);
}

void abre_pedestres()
{
  tempo_espera = 0;
  while(tempo_espera < 2)
    configura_sinal(1,0,0,0,1);
  while(tempo_espera < 4)
     configura_sinal(0,1,0,0,1);
  while(tempo_espera < 6)
    configura_sinal(0,0,1,1,0);
  while(tempo_espera < 8)
    configura_sinal(0,0,1,0,2);
  if(tempo_espera >= 10)
    aberto_carros();
    //configura_sinal(1,0,0,0,1);
}

void pisca_noite()
{
  configura_sinal(0,2,0,0,2);
}

void setup() {
  Timer1.initialize(100000); // Interrupcao a cada 0,1s
  Timer1.attachInterrupt(RSI_timer); // Associa a interrupcao periodica a funcao RSI_timer
  pinMode(pino_botao,INPUT);
  pinMode(verde_carros+5,OUTPUT);
  pinMode(amarelo_carros+5,OUTPUT);
  pinMode(vermelho_carros+5,OUTPUT);
  pinMode(verde_pedestres+5,OUTPUT);
  pinMode(vermelho_pedestres+5,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(2), RSI_botao, RISING);
  //avalia_estado();
}

void loop() {
  //avalia_estado();
  // Entrar em modo SLEEP enquanto nada acontece
  //set_sleep_mode(SLEEP_MODE_IDLE);
  //sleep_enable();
  //sleep_mode();
  //sleep_disable();
}
