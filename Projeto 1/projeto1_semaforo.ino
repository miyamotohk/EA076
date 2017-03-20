#include <TimerOne.h>
#include <avr/sleep.h>

#define verde_carros 0
#define amarelo_carros 1
#define vermelho_carros 2
#define verde_pedestres 3
#define vermelho_pedestres 4

unsigned int tempo_piscar;
int leds[5];

void ISR_timer() {
  tempo_piscar++;
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

void aberto_carros()
{
  leds[0] = 1;
  leds[1] = 0;
  leds[2] = 0;
  leds[3] = 0;
  leds[4] = 1;
  leitura_vetor();
}

void abre_pedestres()
{
  
}

void pisca_noite()
{
  leds[0] = 0;
  leds[1] = 2;
  leds[2] = 0;
  leds[3] = 0;
  leds[4] = 0;
  leitura_vetor();
}

void setup() {
  Timer1.initialize(100000); // Interrupcao a cada 0,1s
  Timer1.attachInterrupt(ISR_timer); // Associa a interrupcao periodica a funcao ISR_timer
  pinMode(verde_carros+5,OUTPUT);
  pinMode(amarelo_carros+5,OUTPUT);
  pinMode(vermelho_carros+5,OUTPUT);
  pinMode(verde_pedestres+5,OUTPUT);
  pinMode(vermelho_pedestres+5,OUTPUT);
}

void loop() {
  
  pisca_noite();
  
  // Entrar em modo SLEEP enquanto nada acontece
  //set_sleep_mode(SLEEP_MODE_IDLE);
  //sleep_enable();
  //sleep_mode();
  //sleep_disable();
}
