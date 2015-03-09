/*
Sinal diseqc em AIN1
*/

//include the library
#include "analogComp.h"

//global variables

boolean
paridade = 0, //paridade calculada
PulseFlag = 0,  //indica deteccao de pulso
Idle = 0,    //inatividade indica fim de transmissao
EnableIdle=1, //habilita mensagem quando detectado pulso
Erro = 0,    //indica deteccao de erro na recepcao
BitVal = 0;  //valor para comparar com paridade calculada

int
PulseCounter = 0,  //contagem de pulsos
ByteVal = 0,      //valor do byte em recebimento
NewByte = 0,      //valor do byte completo
BitCounter = 0,    //bits recebidos
ByteCounter = 0,  //bytes recebidos
Line = 0,    //numero da mensagem
Size = 0,    //bytes recebidos
Counter,    //contador de loop
string[10]; //dados recebidos

unsigned long
PauseStart = 0,  //tempo do ultimo pulso
PauseTime = 0;  //tempo desde o ultimo pulso

//let's set up the hardware
void setup() {
    Serial.begin(9600);
    pinMode(LED13, OUTPUT); //LED pin as output
    analogComparator.setOn(INTERNAL_REFERENCE, AIN1); //referencia interna do comparador (1.1V) sinal=0.65Vpp. Somar 0.76V ao sinal.
    analogComparator.enableInterrupt(changeStatus, RISING); //interrupcao na borda de subida
}

//main loop
void loop() {

  if (PulseFlag){
    Idle=0;
    PauseStart = micros();    //zera contagem da pausa
    PulseFlag=0;
    EnableIdle=1;
  }

  if (Idle) {
      Idle = false;
      Line++;
      if (Line>100) Line=1;
        
      Serial.print(Line);
      Serial.print("[");
      Serial.print(Size);
      if (Erro) {
        Serial.print("Err");
        Erro = 0;
      }
      Serial.print("] - ");

      for(Counter=0;Counter<Size;Counter++){
        Serial.print(string[Counter], HEX);
        Serial.print(" ");
        string[Counter]=0;
      }
      Size=0;
      Serial.println();        
    }


    PauseTime = micros() - PauseStart;

    if((PauseTime > 6000) && (EnableIdle)){  //fim dos dados
      if(ByteCounter>0) Idle=1;
      PulseCounter=0;
      BitCounter=0;
      Size=ByteCounter;
      ByteCounter=0;
      ByteVal=0;
      EnableIdle=0;
      paridade=0;
    }
    else {
      if((PauseTime > 900) && (PulseCounter>=10) && !Idle) {  //bit valor 1
        ByteVal=(ByteVal<<1) | 1;
        BitCounter++;
        if(BitCounter<9){
          paridade=!paridade;
        } else BitVal=1;
        PulseCounter=0;
      } else
      if((PauseTime > 400) && (PulseCounter>=21) && !Idle) {  //bit valor 0
        BitCounter++;        
        BitVal=0;
        ByteVal=(ByteVal<<1) | 0;
        PulseCounter=0;
      }      
      if(BitCounter==8){  //byte completo recebido
        NewByte=ByteVal;
      }

      if(BitCounter==9){  //paridade
        BitCounter=0;
        ByteVal=0;
        string[ByteCounter]=NewByte;
        ByteCounter++;
        if(BitVal==paridade) Erro=1;
        paridade=0;
      }
    }
}

//interrupt to be raised by the analog comparator
void changeStatus() {
  PulseFlag=1;
  PulseCounter++;
  if (PulseCounter > 33){
    PulseCounter = 33;
    paridade=0;
    BitCounter=0;
    ByteCounter=0;
  }
}
