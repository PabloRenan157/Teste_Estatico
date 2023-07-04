#include <SD.h>
#include "EBYTE.h"
#include <max6675.h>
#include <HX711.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define pinoSS 53
#define M0_LoRa   6
#define M1_LoRa   5
#define RX_LoRa   19 // RX1 - Pino 19
#define TX_LoRa   18 // TX1 - Pino 18
#define AUX_LoRa  4

struct DATA {
  unsigned long packet;
  int Bits;
  float noise;
  float buckhead;
  float kg;
};

char ack;
int Chan;
DATA MyData;
unsigned long Last;

EBYTE LoRa(&Serial1, M0_LoRa, M1_LoRa, AUX_LoRa);  // Criar objeto de Transmissão,
MAX6675 temp1(13, 12, 11);
MAX6675 temp2(13, 12, 11);
HX711 escala;

const int LOADCELL_DOUT_PIN = A1;
const int LOADCELL_SCK_PIN = 52;

int porta_rele1 = 7;
int porta_rele2 = 8;
int valueRele = 1;

File dataFile;

void setup() {
  digitalWrite(porta_rele1, HIGH);
  digitalWrite(porta_rele2, HIGH);
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(porta_rele1, OUTPUT);
  pinMode(porta_rele2, OUTPUT);

  initOled();
  initSD();
  initLora();
  SystemsOk();
  

}
void loop() {
  unsigned long startTime = millis();

  while (millis() - startTime < 150) {
    if (Serial1.available()) {
      ack = (char)LoRa.GetByte();

      if (ack == 'A') {
        Serial.println("Received 'A' via LoRa");

        MyData.packet++;
        MyData.noise = readTemperature(1);
        MyData.buckhead = readTemperature(2);
        MyData.kg = readCellLoad();

        valueRele = 2;
        digitalWrite(porta_rele1, LOW);
        MyData.Bits = valueRele;

        LoRa.SendStruct(&MyData, sizeof(MyData));

        Serial.print("Sending a: ");
        Serial.println(MyData.packet);

        // Armazena os dados no cartão SD
        saveDataToFile();

        delay(100);
      }
      else if (ack == 'B') {
        Serial.println("Received 'B' via LoRa");

        MyData.packet++;
        MyData.noise = readTemperature(1);
        MyData.buckhead = readTemperature(2);
        MyData.kg = readCellLoad();

        valueRele = 3;
        digitalWrite(porta_rele2, LOW);
        MyData.Bits = valueRele;
        LoRa.SendStruct(&MyData, sizeof(MyData));

        Serial.print("Sending a: ");
        Serial.println(MyData.packet);

        // Armazena os dados no cartão SD
        saveDataToFile();

        delay(100);
      }
    }
  }

  MyData.packet++;
  MyData.noise = readTemperature(1);
  MyData.buckhead = readTemperature(2);
  MyData.kg = readCellLoad();

  MyData.Bits = valueRele;

  LoRa.SendStruct(&MyData, sizeof(MyData));

  Serial.print("Sending b: ");
  Serial.println(MyData.packet);

  // Armazena os dados no cartão SD
  saveDataToFile();

  delay(150);
}

float readTemperature(int sensorNumber) {
  float temperature = 0.0;

  if (sensorNumber == 1) {
    temperature = random(10, 20);
  // temperature = readCelsius();
  } else if (sensorNumber == 2) {
    temperature = random(30, 40);
  // temperature = 0;
  }

  return temperature;
}

float readCellLoad() {
  float peso = escala.get_units(3);
 if(peso <= 0.0200)
 {
 Serial.print("0");
 return 0;
 }
 else
 {
 Serial.print(peso, 3);
 }
  return peso;
}

void saveDataToFile() {
  dataFile = SD.open("dados.txt", FILE_WRITE);

  if (dataFile) {

    dataFile.print(MyData.packet);
    dataFile.print("| ");

    dataFile.print(MyData.Bits);
    dataFile.print("|");

    dataFile.print(MyData.noise);
    dataFile.print("|");

    dataFile.print(MyData.buckhead);
    dataFile.print("|");

    dataFile.println(MyData.kg);

    dataFile.close();
  }
}

bool initSD() { 
  // Inicialização do cartão microSD
  if (SD.begin(pinoSS)) { // Inicializa o SD Card
    Serial.println("SD Card pronto para uso.");
    display.setCursor(0, 0);
    display.println("SD: OK");
    display.display();
  }
  else {
    Serial.println("Falha na inicialização do SD Card.");
    display.setCursor(0, 0);
    display.println("SD: Fail");
    display.display();
    return;
  }

  // Abre o arquivo para escrita
  dataFile = SD.open("dados.txt", FILE_WRITE);
  
  if (dataFile) {
    // Escreve o cabeçalho no arquivo
    dataFile.println("Packet, Bits, Temp nozzdle (°C), Temp bulkhead (°C), KG");
    dataFile.close();
  } 
  else {
    Serial.println("Não foi possível abrir o arquivo para escrita!");
  }
} 
void initOled(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void initLora()
  {
  if (!LoRa.init()) {
    display.setCursor(0, 10);
    display.println("LoRa: OK");
    display.display();
  } else {
    display.setCursor(0, 10);
    display.println("LoRa: Fail");
    display.display();
  }
  LoRa.SetAirDataRate(ADR_1K);
  LoRa.SetAddress(1);
  LoRa.SetChannel(15);
  LoRa.SaveParameters(TEMPORARY);
  LoRa.PrintParameters();
  LoRa.SetMode(MODE_NORMAL);
  }

 void SystemsOk()
 {
   
  float noiseTemp = readTemperature(1);
  if (noiseTemp > 0) {
    display.setCursor(0, 20);
    display.println("Nozzdle: OK");
    display.display();
  } else {
    display.setCursor(0, 20);
    display.println("Nozzdle: Fail");
    display.display();
  }

  float buckheadTemp = readTemperature(2);
  if (buckheadTemp > 0) {
    display.setCursor(0, 30);
    display.println("Bulkhead: OK");
    display.display();
  } else {
    display.setCursor(0, 30);
    display.println("Bulkhead: Fail");
    display.display();
  }
  display.setCursor(0, 40);
  escala.begin(A1, 52);
  escala.set_scale(2250.28096);
  escala.tare(0);

   if (escala.read()>0) {
    display.println("Cell Load: Ok");
  } else {
    display.println("Cell Load: Fail");
  }
    display.display();
 }
