/* Protocolo de aplicacao - Implementacao usando rotina de interrupcao e
 *  controle.
 *
 *  Uso:
 *  - Computador envia (pelo terminal) uma mensagem:
 *  PING\n\n
 *  - MCU retorna (no terminal):
 *  PONG\n
 *
 *  Tiago F. Tavares
 *  GPL3.0 - 2017
 */

//A biblioteca stdio.h contem rotinas para processamento de expressoes regulares
#include <stdio.h>
#include <Wire.h>

//Definicoes
#define LM35 A0
#define SDA A4
#define SDL A5
#define TAM_MAX_BUFFER 15

//Buffer de dados
typedef struct {
  char data[TAM_MAX_BUFFER];
  unsigned int tam_buffer;
} serial_buffer;

serial_buffer Buffer;

/* Flags globais para controle de processos da interrupcao */
volatile int flag_leu_string = 0;

//Rotina para comparacao de strings. Retorna 1 se sao iguais e 0 caso contrario.
int compara_string(char *string1, char *string2, int tam) {
  int i;
  for (i=0; i<tam; i++) {
    if (string1[i] != string2[i]) return 0;
    if (string1[i] == '\0') return 1;
  }
  return 1;
}

//Limpa o buffer
void limpa_buffer() {
  Buffer.tam_buffer = 0;
}

//Adiciona caractere ao buffer
int insere_buffer(char c_in) {
  if (Buffer.tam_buffer < TAM_MAX_BUFFER) {
    Buffer.data[Buffer.tam_buffer++] = c_in;
    return 1;
  }
  return 0;
}

uint8_t le_temperatura(){
   return (float(analogRead(A0))*5/(1023))/0.01;
}

/* Rotinas de interrupcao */

//RSI de evento da UART
void serialEvent() {
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c=='\n') {
      insere_buffer('\0'); // Se recebeu um fim de linha, coloca um terminador de string no buffer
      flag_leu_string = 1;
    }
    else {
     insere_buffer(c);
    }
  }
}

/* Funcoes internas ao void main() */

void setup() {
  //Inicializacao
  pinMode(LM35, INPUT);
  limpa_buffer();
  flag_leu_string = 0;
  Serial.begin(9600);
}


void loop() {
  char saida_buffer[15];
  int flag_escrita = 0;

  /* A flag_leu_string permite separar a recepcao de caracteres
   *  (vinculada a interrupca) da interpretacao de caracteres. Dessa forma,
   *  mantemos a rotina de interrupcao mais enxuta, enquanto o processo de
   *  interpretacao de comandos - mais lento - nao impede a recepcao de
   *  outros caracteres. Como o processo nao 'prende' a maquina, ele e chamado
   *  de nao-preemptivo.
   */
   
  if (flag_leu_string == 1) {
    if (compara_string(Buffer.data, "PING", 4) ) {
      sprintf(saida_buffer, "PONG\n");
      flag_escrita = 1;
    }

    if (compara_string(Buffer.data, "ID", 2) ) {
      sprintf(saida_buffer, "NOSSO DATALOGGER\n");
      flag_escrita = 1;
    }
    
    if (compara_string(Buffer.data, "MEASURE", 7) ) {
      sprintf(saida_buffer, "%u\n", le_temperatura());
      flag_escrita = 1;
    }
    
    flag_leu_string = 0;
  }

  /* Posso construir uma dessas estruturas if(flag) para cada funcionalidade
   *  do sistema. Nesta a seguir, flag_escrita e habilitada sempre que alguma outra
   *  funcionalidade criou uma requisicao por escrever o conteudo do buffer na
   *  saida UART.
   */
  if (flag_escrita == 1) {
    Serial.write(saida_buffer);
    limpa_buffer();
    flag_escrita = 0;
  }

}
