// INCLUSÃO DE BIBLIOTECAS
#include <Btn.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <HX711_ADC.h>

// DEFINIÇÕES
#define pinRx 10 //Recepctor MP3
#define pinTx 11 //Transmissor MP3
#define volumeMP3 30 //Volume Máximo
#define pinBuzzer 6 //Pino Buzzer
#define pinBtn1 4 //Botão Tara
#define pinBtn2 5 //Botão Resultado
#define pinHX_DT A0 // Pino HX711(Azul)
#define pinHX_SCK A1 //Pino HX711(Branco)
#define pinSensor A3 //Sensor Óptico

//VARIÁVIES
int gotas=0;
int gotasOLD=0;
float volume=0;
float peso=0;
float pesogota=0;
float calibracao=5732.34;
unsigned long t = 0;

// DECLARAÇÃO DE OBJETOS
SoftwareSerial playerMP3Serial(pinRx, pinTx); 
DFRobotDFPlayerMini playerMP3;
HX711_ADC LoadCell(pinHX_DT, pinHX_SCK);

Btn bot1 = new Btn(pinBtn1); //Botão Start - Faz a tara e inicia a contagem
Btn bot2 = new Btn(pinBtn2); //Botão Resultado - Informa o resultado

void setup() {

	Serial.begin(9600);
	playerMP3Serial.begin(9600);
 
  pinMode(pinBtn1, INPUT);
  pinMode(pinBtn2, INPUT);
  pinMode(pinSensor, INPUT);
  pinMode(pinBuzzer, OUTPUT);

  LoadCell.begin(); //Iniciando Balanca
  LoadCell.start(2000, true);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Falha ao iniciar balança - Verifique os pinos");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibracao);
    Serial.println("Balanca ligada");
  }
  Serial.println();

  Serial.println(F("Iniciando Módulo MP3...")); //Iniciando MP3
  if (!playerMP3.begin(playerMP3Serial)) {  //
    Serial.println(F("Falha ao iniciar MP3:"));
    Serial.println(F("1.Confira as conexões!"));
    Serial.println(F("2.Confira o cartão SD!"));
    while(true){
      delay(0);
    }
  }
  Serial.println(F("DFPlayer ligado!"));

  //Configurações iniciais
  playerMP3.setTimeOut(500);
  playerMP3.volume(volumeMP3); // Volume no máximo
  playerMP3.EQ(0); // Equalizacao normal

  playerMP3.playFolder(6, 1); //Iniciar Dosagem
  delay(2000);

}

void loop() {

  //Coleta de dados pela balança
  static boolean newDataReady = 0; 
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) {
      peso = LoadCell.getData(); //Valores da balança armazenada na variável peso
      Serial.println(peso);
      newDataReady = 0;
  }

  //Coleta de dados pelo sensor óptico
  if(analogRead(pinSensor)>200) { //Se gota detectada, então +1 na variável gotas
    gotas++; 
    delay(10); //Delay mínimo apenas para evitar que a mesma gota seja detectada duas vezes durante a queda
  }

  if(gotas>(gotasOLD)) { //Se gota adicionada, então buzzer ligado
    tone(pinBuzzer,450);
    gotasOLD = gotas;
    t=millis();
  }

  if(millis()> t+200) { //Após 200ms, buzzer desligado 
    noTone(pinBuzzer); //Então tempo de duração do buzzer é 200ms, se outra gota cair durante esse tempo o buzzer se mantém tocando por mais 200ms
  }

  bot1.clique(tara); //Botão Tara
  bot2.clique(resultado); //Botão Resultado
  
}

void tara () { //Função Tara - Reseta Variáveis e Realiza a tara da balança
  gotas=0;
  gotasOLD=0;
  volume=0;
  LoadCell.tareNoDelay();
  playerMP3.playFolder(6, 1); // Iniciar Dosagem
  delay(1500);
}

void resultado () { //Função Resultado - Calcula e Reproduz o resultado

  pesogota = peso/gotas; //Calcula o peso unitário de uma gota

  if (pesogota<0.04) { //Se menor que 0.04, então 1ml = 0.97g
    volume = peso*0.97;
  } else if (pesogota<0.05) { //Se menor que 0.05, então 1ml = 1.12g
    volume = peso*1.12;
  } else if (pesogota<0.06) { //Se menor que 0.06, então 1ml = 1.26g
    volume = peso*1.26;
  } else { //Se maior ou igual a 0.06, então 1ml = 1.03g
    volume = peso*1.03;
  }

  int inteiro = volume; //Parte inteiro do volume
  int decimal = (volume-inteiro)*100; //Parte decimal do volume

  //Reprodução Sonora
  //Pasta 1 - Total X gotas e 
  //Pasta 2 - Y ml 
  //Pasta 3 - Y 
  //Pasta 4 - Vírgula zero Z ml
  //Pasta 5 - Vírgula Z ml

  playerMP3.playFolder(1, gotas); //Total X gotas e
  delay(2000);

  if (decimal==0){ //Volume inteiro
    playerMP3.playFolder(2,inteiro); // Y ml
    delay(1000);

  } else if (decimal<10) { //Volume X.01 a X.09
    playerMP3.playFolder(3,inteiro); //Y
    delay(1000);
    playerMP3.playFolder(4,decimal); //Vírgula zero Z ml
    delay(2200);

  } else { //Decimal
    playerMP3.playFolder(3,inteiro); //Inteiro
    delay(1000);
    playerMP3.playFolder(5,decimal); //Vírgula decimal ml
    delay(1700);
  }
}
