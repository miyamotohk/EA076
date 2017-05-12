/* EA076 - Projeto 2: Datalogger
 * Henrique e Daniel - Turma D
 *
 * Esse e o codigo para um sistema datalogger que realiza usa um sensor de temperatura para fazer a leitura da temperatura do ambiente.
 * O sistema utiliza comunicacao RS232 (serial) para interface com o computador, utiliza comunicacao I2C para comunicacao comm uma memoria EEPROM (AT24C16), 
 * onde os dados sao gravados e dispoe de um sistema matriciail, que tambem pode controlar o sistema.
 * E possivel solicitar medicoes e gravacoes de apenas um dado atraves do computador ou do teclado matricial. Ou entao o sistema dispoe de modo de medicao automatica
 * controlado pelo teclado matricial, que realiza medicoes com frequencia definida.
 * 
 * Codigo parcialmente baseado no codigo de Kernel de tempo real e comunicacao UART de Tiago Tavares.
 * 
 */

//Inclusao de bibliotecas
#include <stdio.h>
#include <Wire.h>
#include <TimerOne.h>

//Definicoes de pinos e tamanho maximo do buffer
#define LM35 A0
#define C1 2
#define C2 3
#define C3 4
#define L1 5
#define L2 6
#define L3 7
#define L4 8
#define LED 10
#define TAM_MAX_BUFFER 25

//Vetores de linhas e colunas do teclado matricial
int linha[4] = {L1,L2,L3,L4};
int coluna[3] = {C1,C2,C3};

//Matriz de teclas do teclado matricial
char teclado[4][3] = {{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};

//Endereco da EEPROM
const byte END_EEPROM = 0x50;

//Tipo buffer (usado para comunicacao serial e com teclado matricial)
typedef struct {
  char data[TAM_MAX_BUFFER];
  unsigned int tam_buffer;
} buffer;

//Variaveis do tipo buffer
buffer buffer_serial;
buffer buffer_teclado;

//Buffer de saida (escrita na serial)
char saida_buffer[15];

//Variaveis globais para controle de processos
volatile int flag_leu_string = 0;
volatile int flag_deboucing = 0;
volatile int flag_leu_teclado = 0;
volatile int flag_medicao_automatica = 0;
volatile int flag_escrita = 0;
volatile int tempo_deboucing;
volatile unsigned long int contador = 0;

//Rotina para comparacao de strings. Retorna 1 se sao iguais e 0 caso contrario.
int compara_string(char *string1, char *string2, int tam) {
  int i;
  for (i=0; i<tam; i++){
    if (string1[i] != string2[i]) return 0;
    if (string1[i] == '\0') return 1;
  }
  return 1;
}

//FUNCOES DE BUFFER
//Limpa o buffer serial
void limpa_Sbuffer() {
  buffer_serial.tam_buffer = 0;
}

//Limpa o buffer teclado
void limpa_Tbuffer() {
  buffer_teclado.tam_buffer = 0;
}

//Adiciona caractere ao buffer serial
int insere_Sbuffer(char c_in) {
  if (buffer_serial.tam_buffer < TAM_MAX_BUFFER) {
    buffer_serial.data[buffer_serial.tam_buffer++] = toupper(c_in);
    return 1;
  }
  return 0;
}

//Adiciona caractere ao buffer teclado
int insere_Tbuffer(char c_in) {
  if (c_in == '*')
    flag_leu_teclado = 1;
  if (buffer_teclado.tam_buffer < TAM_MAX_BUFFER) {
    buffer_teclado.data[buffer_teclado.tam_buffer++] = c_in;
    return 1;
  }
  return 0;
}

//FUNCOES DE MEMORIA
//Escrita de um byte da memoria
void escreve_byte(byte end_eeprom, unsigned end_mem, byte dado){
  byte aux = end_eeprom | ((end_mem >> 8) & 0x07);
  
  Wire.beginTransmission(aux); //Inicia transmissao
  Wire.write(int(end_mem)); //Envia endereco de memoria
  Wire.write(dado); //Envia dado a ser escrito
  Wire.endTransmission(); //Fim da transmissao
  delay(100);
}

//Leitura de um byte da memoria
int le_byte(byte end_eeprom, unsigned end_mem){
  byte dado = -1;
  byte aux = end_eeprom | ((end_mem >> 8) & 0x07);
  
  Wire.beginTransmission(aux); //Inicia transmissao
  Wire.write(int(end_mem)); //Envia endereco de memoria
  Wire.endTransmission(); //Fim da transmissao
  Wire.requestFrom(int(aux),1); //Requisita dados
  if(Wire.available())
    dado = Wire.read(); //Leitura do dado
  delay(100);
  return dado;
}

//FUNCOES DE LEITURA DE DADOS DO SENSOR
//Leitura da temperatura
uint8_t le_temperatura(){
   return (float(analogRead(A0))*5/(1023))/0.01; //Converte o valor da leitura para o valor de temperatura em Celsius
}

//Realiza uma medicao e grava o valor na memoria
void grava_medicao(){
    int N = le_byte(END_EEPROM, 0x00); //Le status da memoria (valor de N)
    int dado = le_temperatura(); //Le dado (valor do sensor)
    escreve_byte(END_EEPROM, (N+1), byte(dado)); //Escreve dado na posicao de memoria N+1
    N++; //Incrementa N
    escreve_byte(END_EEPROM, 0x00, byte(N)); //Atualiza valor de N na memoria
    sprintf(saida_buffer, "Dado gravado!\n"); //Escreve na serial
    flag_escrita = 1;
}

//FUNCOES DO TECLADO MATRICIAL
//Funcao de varredura do teclado matricial
char varredura(){
  int i, j;
  for(i=0; i<4; i++){
    //Coloca todas as linhas em alto
    digitalWrite(linha[0], HIGH);
    digitalWrite(linha[1], HIGH);
    digitalWrite(linha[2], HIGH);
    digitalWrite(linha[3], HIGH);
    //Coloca a i-esima linha em baixo
    digitalWrite(linha[i], LOW);
    for(j=0; j<3; j++){
      if(digitalRead(coluna[j]) == LOW){ //Se a leitura da j-esima coluna retornar baixo
        return teclado[i][j]; //Retorna valor (i,j) do teclado
      }
    }
    digitalWrite(linha[i], HIGH); //Retorna a i-esima linha para alto
  }
}

//ROTINAS DE INTERRUPCAO
//RSI de evento da UART
void serialEvent() {
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c=='\n') { //Se a string terminou, colocar terminador de string no buffer
      insere_Sbuffer('\0');
      flag_leu_string = 1;
    }
    else { //Se a string ainda nao terminou, insere caractere no buffer
     insere_Sbuffer(c);
    }
  }
}

//Interrupcao periodica
void RSI_timer(){
  if(flag_medicao_automatica == 1) //Se a medicao automatica estiver ativa, incrementa contador
    contador ++;
  
  if(flag_deboucing == 0){ //Nao estou em deboucing
    char c = varredura();
    //Verifica se c e um caractere valido
    if((c=='0')||(c=='1')||(c=='2')||(c=='3')||(c=='4')||(c=='5')||(c=='6')||(c=='7')||(c=='8')||(c=='9')||(c=='*')||(c=='#')){
      insere_Tbuffer(c); //Insere caratere no buffer
      flag_deboucing = 1; //Ativa deboucing
      tempo_deboucing = 500;
    }
  }
  else{ //Estou em deboucing
    tempo_deboucing--; //Decrementa tempo de deboucing
    if(tempo_deboucing == 0)
      flag_deboucing = 0; //Desativa deboucing
  }
  
}

void setup() {
  //Configura pinos como entrada/saida
  pinMode(LM35, INPUT);
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);
  pinMode(LED, OUTPUT);
  
  Timer1.initialize(1000); //Configura timer com periodo 1ms (1kHz)
  Timer1.attachInterrupt(RSI_timer); //Associa a interrupcao periodica a funcao RSI_timer
  
  limpa_Sbuffer(); //Limpa buffer serial
  limpa_Tbuffer(); //Limpa buffer do teclado
  
  //Inicia flags
  flag_leu_string = 0;
  flag_deboucing = 0;
  flag_leu_teclado = 0;
  flag_medicao_automatica = 0;
  flag_escrita = 0;
  
  delay(500); //Delay para inicio da comunicacao serial
  Serial.begin(9600); //Inicia comunicacao serial
  Wire.begin(); //Inicia comunicacao I2C com EEPROM
}

void loop() {
  
  if (flag_leu_string == 1) { //Se uma string foi enviada pela serial
  
    if (compara_string(buffer_serial.data, "PING", 4) ) { //Se recebeu mensagem "PING"
      sprintf(saida_buffer, "PONG\n"); //Responde "PONG"
      flag_escrita = 1;
    }

    if (compara_string(buffer_serial.data, "ID", 2) ) { //Se recebeu mensagem "ID"
      sprintf(saida_buffer, "INTO THE WILD DATALOGGER\n"); //Responde com nome do datalogger
      flag_escrita = 1;
    }
    
    if (compara_string(buffer_serial.data, "MEASURE", 7) ) { //Se recebeu mensagem "MEASURE"
      sprintf(saida_buffer, "%u\n", le_temperatura()); //Responde com a leitura do sensor
      flag_escrita = 1;
    }
    if (compara_string(buffer_serial.data, "MEMSTATUS", 9) ) { //Se recebeu mensagem "MEMSTATUS"
      int memstatus = le_byte(END_EEPROM, 0x00);
      sprintf(saida_buffer, "%u\n", memstatus); //Responde com o valor de N (posicao 0 de memoria)
      flag_escrita = 1;
    }
    if (compara_string(buffer_serial.data, "RESET", 5) ) { //Se recebeu mensageem "RESET"
      escreve_byte(END_EEPROM, 0x00, 0x00); //Coloca 0 em N (posicao 0 de memoria)
      sprintf(saida_buffer, "Memoria resetada!\n"); //Responde texto
      flag_escrita = 1;
    }
    if (compara_string(buffer_serial.data, "RECORD", 6) ) { //Se recebeu mensagem "RECORD"
      grava_medicao(); //Realiza uma medicao e grava o valor na memoria
    }
    if (compara_string(buffer_serial.data, "GET", 3) ) { //Se recebeu mensagem "GET"
      int N = 0;
      sscanf(buffer_serial.data, "%*s %d", &N); //Le valor de N (posicao de memoria) desejado
      int dado = le_byte(END_EEPROM, N); //Recupera valor da N-esima posicao de memoria
      sprintf(saida_buffer, "%d\n", dado); //Responde com tal valor de dado
      flag_escrita = 1;
    }
    
    flag_leu_string = 0;
  }
  
  if(flag_leu_teclado == 1){
    if (compara_string(buffer_teclado.data, "#1*", 3) ) { //Se recebeu comando #1*
      //Pisca LED
      for(int i=0; i<6; i++){
        if(i%2){
          digitalWrite(LED,HIGH);
          delay(100);
        }
        else{
          digitalWrite(LED,LOW);
          delay(100);
        }
      digitalWrite(LED,LOW);
      }
    }
    if (compara_string(buffer_teclado.data, "#2*", 3) ) { //Se recebeu comando #2*
      grava_medicao(); //Realiza uma medicao e grava o valor na memoria
    }
    if (compara_string(buffer_teclado.data, "#3*", 3) ) { //Se recebeu comando #3*
      flag_medicao_automatica = 1; //Ativa medicao automatica
      sprintf(saida_buffer, "Ativa medicao automatica\n"); //Escreve na serial
      flag_escrita = 1;
    }
    if (compara_string(buffer_teclado.data, "#4*", 3) ) { //Se recebeu comando #4*
      flag_medicao_automatica = 0; //Desativa medicao automatica
      sprintf(saida_buffer, "Desativa medicao automatica\n"); //Escreve na serial
      flag_escrita = 1;    
    }
    
    limpa_Tbuffer();
    flag_leu_teclado = 0;
  }
  
  
  //Estruturas if(flag) para diferentes funcionalidades do sistema
  
  //flag_escrita controla a escrita atraves da serial
  if (flag_escrita == 1) {
    Serial.write(saida_buffer);
    limpa_Sbuffer();
    flag_escrita = 0;
  }
  
  //flag_medicao_automatica controla a medicao automatica do sensor, a cada 5s
  if (flag_medicao_automatica == 1){
    if(contador > 5000){ //Se contador > 5s
      grava_medicao(); //Realiza medicao e grava na memoria
      contador = 0;
    }
  }

}
