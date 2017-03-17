#include <TimerOne.h>
#include <avr/sleep.h>

#define verde_carros 5
#define amarelo_carros 6
#define vermelho_carros 7
#define vermelho_pedestres 8
#define verde_pedestres 9

unsigned int tempo;
int estado = 0;
int leds[5];

void ISR_timer() {
  tempo++;
}

void leitura_vetor(){
  for(int i=0; i<5; i++){
    if(leds[i]==0)
      digitalWrite(i+5,LOW);
    else if(leds[i]==1)
      digitalWrite(i+5,HIGH);
    else if(leds[i]==2){
      if(tempo%10 < 5)
        digitalWrite(i+5,HIGH);
      else
        digitalWrite(i+5,LOW);
    }
  }
}

void setup() {
  Timer1.initialize(10000000); // Interrupcao a cada 0,1s
  Timer1.attachInterrupt(ISR_timer); // Associa a interrupcao periodica a funcao ISR_timer
  pinMode(verde_carros,OUTPUT);
  pinMode(amarelo_carros,OUTPUT);
  pinMode(vermelho_carros,OUTPUT);
  pinMode(verde_pedestres,OUTPUT);
  pinMode(vermelho_pedestres,OUTPUT);
}

void loop() {
  for(int i=0; i<5; i++)
    leds[i]=2;
  leitura_vetor();
  /*delay(500);
  for(int i=0; i<5; i++)
    leds[i]=1;
  leitura_vetor();
  delay(500);*/
  
  // Entrar em modo SLEEP enquanto nada acontece
  //set_sleep_mode(SLEEP_MODE_IDLE);
  //sleep_enable();
  //sleep_mode();
  //sleep_disable();
}
