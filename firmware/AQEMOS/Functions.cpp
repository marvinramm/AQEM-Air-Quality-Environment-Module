// AQEMOS - firmware for the AQEM Air Quality Environment Module
// Copyright (c) 2025 Marvin Ramm - PolyForm Noncommercial 1.0.0, see LICENSING.md

///////////////////Includes//////////////////

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "Measure.h"
#include "Pins.h"
#include "Settings.h"
#include "Functions.h"
#include "ESP32Zigbee.h"

/////////////////////////////////////////////

//////////////////Variables//////////////////

/////////////////////////////////////////////

////////////////////Code/////////////////////

void HardwareInit() {
  Serial.begin(115200);

  Serial1.begin(9600, SERIAL_8N1, RXUART, TXUART);  //UART PMS5003

  Serial.println("AQEM OS Init...");

  uint64_t chipid = ESP.getEfuseMac();
  char chipIDStr[13];
  sprintf(chipIDStr, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);

  Serial.print("ChipID: ");
  Serial.println(chipIDStr);

  Wire.begin(SDA, SCL);

  //Output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, LOW);

  //Input
  pinMode(MICAO, INPUT);
  pinMode(G_INT, INPUT_PULLUP);
  pinMode(LIGHT, INPUT);
  pinMode(BOOT, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(G_INT), handleGeigerPulse, CHANGE);

  if (BME680Init() == false) {
    Serial.println("BME680 = OFFLINE");
  } else {
    Serial.println("BME680 = ONLINE");
  }

  if (SCD40Init() == false) {
    Serial.println("SCD40 = OFFLINE");
  } else {
    Serial.println("SCD40 = ONLINE");
  }

  if (SGP30Init() == false) {
    Serial.println("SGP30 = OFFLINE");
  } else {
    Serial.println("SGP30 = ONLINE");
  }

  MQ7Init();

  if (EN_ZIGBEE == 1) {
    ZigbeeInit();
  }

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
}

/////////////////////////////////////////////
