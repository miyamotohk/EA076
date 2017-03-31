/*
 EA076 - Projeto 1: Semaforo
 Henrique e Daniel - Turma D
 
 Esse projeto e um prototipo de semaforo, baseado em uma maquina de estados.
 O sistema esta normalmente aberto para os carros (estado 0).
 Caso o botao seja pressionado (por um pedestre), transiciona para o estado 1, em que fecha para carros, abre para pedestres e volta a abrir para carros.
 Se o sensor de luminosidade detectar iluminacao baixa por certo periodo de tempo continuo (noite), transiciona para o estado 2, em que pisca o sinal amarelo para carros e vermelho para pedestres.
 */

//Inclusao de bibliotecas
#include <TimerOne.h>
#include <avr/sleep.h>

//Definicoes de valores de pinos
#define verde_carros 0
#define amarelo_carros 1
#define vermelho_carros 2
#define verde_pedestres 3
#define vermelho_pedestres 4
#define pino_botao 2
#define pino_LDR A0

//Variaveis de contagem de tempo
unsigned int tempo_piscar;
unsigned int tempo_espera;
unsigned int tempo_noite;

int conta_tempo_noite = 0; //Variavel que habilita incremento de tempo_noite
volatile int estado = 0; //Variavel que define estado da maquina de estados
int leds[5]; //Vetor que contem informacoes sobre o estado de cada LED

void RSI_timer() //RSI periodica (T = 0.1s)
{
  tempo_piscar++;
  if((!(tempo_piscar%10))&&(estado==1)) //Contador de período 1s, caso esteja no estado 1
    tempo_espera++;
  if((!(tempo_piscar%10))&&(conta_tempo_noite)) //Contador de período 1s, caso esteja habilitado atraves da flag
    tempo_noite++;
  
  if((analogRead(pino_LDR) > 300)&&(estado!=1)) //Se estiver escuro (e fora do estado 1)
  {
    conta_tempo_noite = 1; //Conta tempo que permanece assim
    if(tempo_noite > 5) //Se permanecer por 5s
    {
      //Configura estado 2 para piscar sinal de alerta a noite
      estado = 2;
      conta_tempo_noite = 0;
      tempo_noite = 0;
    }
  }
  if((analogRead(pino_LDR) < 300)&&(estado!=1)) //Se estiver claro (e fora do estado 1)
  {
    conta_tempo_noite = 1; //Conta tempo que permanece assim
    if(tempo_noite > 5) //Se permanecer por 5s
    {
      //Configura estado 0 para abrir para carros
      estado = 0;
      conta_tempo_noite = 0;
      tempo_noite = 0;
    }
  }

}   

void RSI_botao() //RSI associada ao acionamento do botao
{
  if(estado==0) //Faz a transicao do estado 0 para 1
    estado = 1;
}

void configura_sinal(int a, int b, int c, int d, int e) //Funcao que configura os LEDs
{
  //Insere no vetor leds[] informacao referente a cada LED
  leds[verde_carros] = a;
  leds[amarelo_carros] = b;
  leds[vermelho_carros] = c;
  leds[verde_pedestres] = d;
  leds[vermelho_pedestres] = e;
  leitura_vetor(); //Configura os LEDs do circuito fisico de acordo com as informacoes do vetor
}

void leitura_vetor(){ //Funcao que configura o i-esimo LED em funcao do i-esimo valor do vetor leds[]
  for(int i=0; i<5; i++){
    if(leds[i]==0) //Valor 0: LED apagado
      digitalWrite(i+5,LOW);
    else if(leds[i]==1) //Valor 1: LED aceso
      digitalWrite(i+5,HIGH);
    else if(leds[i]==2){ //Valor 2: LED pisca com periodo 1s
      if(tempo_piscar%10 < 5)
        digitalWrite(i+5,HIGH);
      else
        digitalWrite(i+5,LOW);
    }
  }
}

void avalia_estado() //Funcao que chama a funcao referente ao estado em que o sistema se encontra
{
  if(estado==0) //Estado 0: sinal aberto para os carros
    aberto_carros();
  else if(estado==1) //Estado 1: abre momentaneamente para os pedestres
    abre_pedestres();
  else if(estado==2) //Estado 2: pisca sinal de alerta durante a noite
    pisca_noite();

}

void aberto_carros() //Aciona sinal verde para carros e vermelho para pedestres
{
  configura_sinal(1,0,0,0,1);
}

void abre_pedestres() //Abre para os pedestres e fecha novamente, passo a passo
{
  if(tempo_espera < 2) //Espera com sinal aberto para carros
    configura_sinal(1,0,0,0,1);
  else if(tempo_espera < 4) //Aciona sinal amarelo para carros
     configura_sinal(0,1,0,0,1);
  else if(tempo_espera < 6) //Fecha sinal para carros e abre para pedestres
    configura_sinal(0,0,1,1,0);
  else if(tempo_espera < 8) //Pisca sinal vermelho para pedestres
    configura_sinal(0,0,1,0,2);
  else if(tempo_espera >= 10) //Retorna ao estado 0
  {
    estado = 0;
    tempo_espera = 0;
  }
}

void pisca_noite() //Pisca sinal amarelo para carros e vermelho para pedestres
{
  configura_sinal(0,2,0,0,2);
}

void setup() {
  Timer1.initialize(100000); //Configura timer com periodo 0,1s
  Timer1.attachInterrupt(RSI_timer); //Associa a interrupcao periodica a funcao RSI_timer
  attachInterrupt(digitalPinToInterrupt(2), RSI_botao, RISING); //Associa interrupcao por acionamento do botao a funcao RSI_botao
  //Configura pinos de entrada (botao e LDR)
  pinMode(pino_botao,INPUT);
  pinMode(pino_LDR,INPUT);
  //Configura pinos de saida (LEDs)
  pinMode(verde_carros+5,OUTPUT);
  pinMode(amarelo_carros+5,OUTPUT);
  pinMode(vermelho_carros+5,OUTPUT);
  pinMode(verde_pedestres+5,OUTPUT);
  pinMode(vermelho_pedestres+5,OUTPUT);
}

void loop() {
  avalia_estado();
}
