#include <SoftwareSerial.h>
#include "EBYTE.h"

#define M0_LoRa   4
#define M1_LoRa   5
#define RX_LoRa   3               // Vai no TXD do módulo
#define TX_LoRa   2               // Vai no RXD do módulo
#define AUX_LoRa  6

struct DATA {
  unsigned long packet;
  int Bits;
  float noise;
  float buckhead;
  float kg;
};

int Chan;
DATA MyData;
unsigned long Last;
unsigned long RequestTime;

SoftwareSerial lora(RX_LoRa, TX_LoRa);
EBYTE LoRa(&lora, M0_LoRa, M1_LoRa, AUX_LoRa);

void setup() {
  Serial.begin(115200);
  lora.begin(9600);
  LoRa.init();
  LoRa.SetAirDataRate(ADR_1K);
  LoRa.SetAddress(1);
  LoRa.SetChannel(15);
  LoRa.SaveParameters(TEMPORARY);
 // LoRa.PrintParameters();
  LoRa.SetMode(MODE_NORMAL);
}

void loop() {
  if (Serial.available()) {
    char receivedChar = Serial.read();
    if (receivedChar == 'a') {
      LoRa.SendByte('A');
      delay(200);
      LoRa.SendByte('A');
      delay(200);
      LoRa.SendByte('A');
      delay(200);
      LoRa.SendByte('A');
      RequestTime = millis();
    }
    else if (receivedChar == 'b') {
      LoRa.SendByte('B');
      delay(200);
      LoRa.SendByte('B');
      delay(200);
      LoRa.SendByte('B');
      delay(200);
      LoRa.SendByte('B');
      RequestTime = millis();
    }
  }

  if (lora.available()) {
    LoRa.GetStruct(&MyData, sizeof(MyData));

   // Serial.print("Packet: ");
    Serial.println(MyData.packet);
    //Serial.print("Bits: ");
    Serial.println(MyData.Bits);
    //Serial.print("Noise: ");
    Serial.println(MyData.noise);
   // Serial.print("Buckhead: ");
    Serial.println(MyData.buckhead);
   // Serial.print("Kg: ");
    Serial.println(MyData.kg);

    unsigned long ResponseTime = millis() - RequestTime;
   // Serial.print("Tempo de resposta: ");
    Serial.println(ResponseTime);
   // Last = millis();
    RequestTime = millis();
  }
}
