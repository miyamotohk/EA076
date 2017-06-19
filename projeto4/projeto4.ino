//Inclusao de biblitoecas
#include<Wire.h>
#include<TimerOne.h>

//Definicoes de frequencias de algumas notas
#define NOTE_C4  262
#define NOTE_DS4 311
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_AS4 466

//Definicoes e constantes
#define pino_botao 2
#define pino_som 8
const unsigned int f_min = 131;
const unsigned int f_max = 494;
const int MPU=0x68;  //Endereco I2C do MPU6050

//Variaveis globais para controle de processos
int AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ; //Variaveis para armazenar valores do sensor
bool estado = 0;                 //Variaval que indica estado (modo de operacao)
int tempo_deboucing = 0;         //Contador de tempo do deboucing
int flag_deboucing = 0;          //Flag que indica modo de deboucing ativo

//Rotinas de servico de interrupcao
void RSI_timer(){
  if(flag_deboucing == 1){       //Se estiver em deboucing
    tempo_deboucing--;           //Decrenenta contador debocuing
    if(tempo_deboucing == 0)     //Ao fim do tempo, desabilita deboucing
      flag_deboucing = 0;
  }
}

void RSI_botao(){
  if(flag_deboucing == 0){      //Se nao estiver em deboucing
    estado = !estado;           //Troca estado
    flag_deboucing = 1;         //Ativa deboucing
    tempo_deboucing = 500;
  }
}

void setup()
{
  //Configura pinos de entrada e saida
  pinMode(pino_botao,INPUT);
  pinMode(pino_som,OUTPUT);
  //Inicializacao do acelerometro (I2C)
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0); 
  Wire.endTransmission(true);
  //Configura interrupcao do botao
  attachInterrupt(0,RSI_botao,RISING);
  //Configura interrupcao periodica
  Timer1.initialize(1000); //Configura timer com periodo 1ms (1kHz)
  Timer1.attachInterrupt(RSI_timer); //Associa a interrupcao periodica a funcao RSI_timer
}

void loop()
{   
  //Inicia comunicacao com sensor para solicitar dados
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);              //Inicia com registrador 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true); //Solicita os dados do sensor  
  
  //Armazena o valor dos sensores nas variaveis correspondentes
  AcX=Wire.read()<<8|Wire.read();  //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY=Wire.read()<<8|Wire.read();  //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  
  if(estado == 0){ //Se estiver no estado 0 (modo continuo - variacao linear de frequencia)
    int S = AcX + AcY + AcZ;
    if(S > 30000)
      tone(pino_som,494);
      //tone(pino_som,f_max);
    else if(S < -30000)
      tone(pino_som,131);
      //tone(pino_som,f_min);
    else{
      int f = (0.00605*S) + 312.5;
      ///unsigend int f = (((f_max - f_min)/(60000))*S) + ((f_max + f_min)/2);
      tone(pino_som,f);
    }
  }
  
  else{ //Se estiver no estado 1 (modo discreto - cada posicao corresponde a uma nota)
    if(AcX < -13000)
      tone(pino_som,NOTE_C4);
    else if(AcZ > 13000)
      tone(pino_som,NOTE_DS4);
    else if(AcX > 13000)
      tone(pino_som,NOTE_F4);
    else if(AcY < -13000)
      tone(pino_som,NOTE_FS4);
    else if(AcY > 13000)
      tone(pino_som,NOTE_AS4);
  }
     
}
