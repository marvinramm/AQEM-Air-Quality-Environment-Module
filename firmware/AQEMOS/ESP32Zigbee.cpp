// AQEMOS - firmware for the AQEM Air Quality Environment Module
// Copyright (c) 2025 Marvin Ramm - PolyForm Noncommercial 1.0.0, see LICENSING.md

///////////////////Includes//////////////////

#include <Arduino.h>
#include "Zigbee.h"

#include "Measure.h"
#include "Pins.h"
#include "Settings.h"
#include "Functions.h"
#include "ESP32Zigbee.h"


#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#define CARBON_DIOXIDE_SENSOR_ENDPOINT_NUMBER 10
#define PM2_5_SENSOR_ENDPOINT_NUMBER 1
#define ZIGBEE_ILLUMINANCE_SENSOR_ENDPOINT 9
#define TEMP_SENSOR_ENDPOINT_NUMBER 8

#define PM1_ANALOG_ENDPOINT_NUMBER 11
#define PM10_ANALOG_ENDPOINT_NUMBER 12
#define RADIATION_ANALOG_ENDPOINT_NUMBER 13
#define NOISE_ANALOG_ENDPOINT_NUMBER 14
#define VOC_ANALOG_ENDPOINT_NUMBER 15
#define CO_ANALOG_ENDPOINT_NUMBER 16

ZigbeeCarbonDioxideSensor zbCarbonDioxideSensor = ZigbeeCarbonDioxideSensor(CARBON_DIOXIDE_SENSOR_ENDPOINT_NUMBER);
ZigbeePM25Sensor zbPM25Sensor = ZigbeePM25Sensor(PM2_5_SENSOR_ENDPOINT_NUMBER);
ZigbeeIlluminanceSensor zbIlluminanceSensor = ZigbeeIlluminanceSensor(ZIGBEE_ILLUMINANCE_SENSOR_ENDPOINT);
ZigbeeTempSensor zbTempSensor = ZigbeeTempSensor(TEMP_SENSOR_ENDPOINT_NUMBER);

//Analog endpoints
ZigbeeAnalog zbPM1Analog = ZigbeeAnalog(PM1_ANALOG_ENDPOINT_NUMBER);
ZigbeeAnalog zbPM10Analog = ZigbeeAnalog(PM10_ANALOG_ENDPOINT_NUMBER);
ZigbeeAnalog zbRadiationAnalog = ZigbeeAnalog(RADIATION_ANALOG_ENDPOINT_NUMBER);
ZigbeeAnalog zbNoiseAnalog = ZigbeeAnalog(NOISE_ANALOG_ENDPOINT_NUMBER);
ZigbeeAnalog zbVOCAnalog = ZigbeeAnalog(VOC_ANALOG_ENDPOINT_NUMBER);
ZigbeeAnalog zbCOAnalog = ZigbeeAnalog(CO_ANALOG_ENDPOINT_NUMBER);

/////////////////////////////////////////////

//////////////////Variables//////////////////

/////////////////////////////////////////////

////////////////////Code/////////////////////

void ZigbeeInit() {

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  // --- read chip ID ---
  uint64_t chipid = ESP.getEfuseMac();
  char chipIDStr[13];
  sprintf(chipIDStr, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);

  // --- Zigbee model ID with chip ID ---
  char deviceName[32];
  snprintf(deviceName, sizeof(deviceName), "AQEM_%s", chipIDStr);

  zbCarbonDioxideSensor.setManufacturerAndModel("Espressif", deviceName);
  zbIlluminanceSensor.setPowerSource(ZB_POWER_SOURCE_MAINS);

  zbCarbonDioxideSensor.setMinMaxValue(0, 3000);
  zbPM25Sensor.setMinMaxValue(0, 350);
  zbPM25Sensor.setTolerance(1);
  zbIlluminanceSensor.setMinMaxValue(0, 10000);
  zbIlluminanceSensor.setTolerance(1);
  zbTempSensor.setMinMaxValue(10, 50);
  zbTempSensor.setTolerance(1);
  zbTempSensor.addHumiditySensor(0, 100, 1);

  // --- PM1 analog
  zbPM1Analog.addAnalogInput();
  zbPM1Analog.setAnalogInputApplication(ESP_ZB_ZCL_AI_RPM_OTHER);
  zbPM1Analog.setAnalogInputDescription("PM1");
  zbPM1Analog.setAnalogInputResolution(1);

  // --- PM10 analog
  zbPM10Analog.addAnalogInput();
  zbPM10Analog.setAnalogInputApplication(ESP_ZB_ZCL_AI_RPM_OTHER);
  zbPM10Analog.setAnalogInputDescription("PM10");
  zbPM10Analog.setAnalogInputResolution(1);

  // --- Radiation uSv/h analog
  zbRadiationAnalog.addAnalogInput();
  zbRadiationAnalog.setAnalogInputApplication(ESP_ZB_ZCL_AI_RPM_OTHER);
  zbRadiationAnalog.setAnalogInputDescription("dose");
  zbRadiationAnalog.setAnalogInputResolution(0.1);

  // --- Noise [dB] analog
  zbNoiseAnalog.addAnalogInput();
  zbNoiseAnalog.setAnalogInputApplication(ESP_ZB_ZCL_AI_RPM_OTHER);
  zbNoiseAnalog.setAnalogInputDescription("noise");
  zbNoiseAnalog.setAnalogInputResolution(1);

  // --- TVOC [ppb] analog
  zbVOCAnalog.addAnalogInput();
  zbVOCAnalog.setAnalogInputApplication(ESP_ZB_ZCL_AI_RPM_OTHER);
  zbVOCAnalog.setAnalogInputDescription("voc");
  zbVOCAnalog.setAnalogInputResolution(1);

  // --- CO [ppm] analog
  zbCOAnalog.addAnalogInput();
  zbCOAnalog.setAnalogInputApplication(ESP_ZB_ZCL_AI_RPM_OTHER);
  zbCOAnalog.setAnalogInputDescription("co");
  zbCOAnalog.setAnalogInputResolution(1);  // 1 ppm resolution

  Zigbee.addEndpoint(&zbCarbonDioxideSensor);
  Zigbee.addEndpoint(&zbPM25Sensor);
  Zigbee.addEndpoint(&zbIlluminanceSensor);
  Zigbee.addEndpoint(&zbTempSensor);

  Zigbee.addEndpoint(&zbPM1Analog);
  Zigbee.addEndpoint(&zbPM10Analog);
  Zigbee.addEndpoint(&zbRadiationAnalog);
  Zigbee.addEndpoint(&zbNoiseAnalog);
  Zigbee.addEndpoint(&zbVOCAnalog);
  Zigbee.addEndpoint(&zbCOAnalog);

  Serial.println("Starting Zigbee...");
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start! Rebooting...");
    ESP.restart();
  }
  Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    digitalWrite(LED_RED, LOW);
    delay(250);
    digitalWrite(LED_RED, HIGH);
    delay(250);
  }
  Serial.println("\nConnected");

  zbCarbonDioxideSensor.setReporting(60, 300, 50);
  zbPM25Sensor.setReporting(60, 300, 1);
  zbIlluminanceSensor.setReporting(60, 300, 5);
  zbTempSensor.setReporting(30, 300, 0);
}

void ReadPairButton() {
  if (digitalRead(BOOT) == LOW) {
    delay(100);
    int startTime = millis();
    while (digitalRead(BOOT) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        Serial.println("Resetting Zigbee to factory and rebooting in 1s...");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
    zbCarbonDioxideSensor.report();
    zbPM25Sensor.report();
    zbIlluminanceSensor.report();
    zbTempSensor.report();

    zbPM1Analog.reportAnalogInput();
    zbPM10Analog.reportAnalogInput();
    zbRadiationAnalog.reportAnalogInput();
    zbNoiseAnalog.reportAnalogInput();
    zbVOCAnalog.reportAnalogInput();
    zbCOAnalog.reportAnalogInput();
  }
}

void UpdateEndpoint() {
  zbCarbonDioxideSensor.setCarbonDioxide(getAverageSCD40CO2());
  zbPM25Sensor.setPM25(getAveragePM2_5());

  int rawLight = analogRead(LIGHT);
  int lux = map(rawLight, 0, 3200, 0, 10000);  // approximate lux scale, not calibrated
  zbIlluminanceSensor.setIlluminance(lux);
  zbTempSensor.setTemperature((getAverageBME680Temp() + getAverageSCD40Temp()) / 2);
  zbTempSensor.setHumidity((getAverageBME680Humidity() + getAverageSCD40Humidity()) / 2);

  zbPM1Analog.setAnalogInput(getAveragePM1_0());
  zbPM10Analog.setAnalogInput(getAveragePM10());

  float roundedDose = round(doseRate_uSv * 100.0) / 100.0;
  zbRadiationAnalog.setAnalogInput(roundedDose);
  zbNoiseAnalog.setAnalogInput(getAverageNoise());
  zbVOCAnalog.setAnalogInput(getAverageSGP30TVOC());
  zbCOAnalog.setAnalogInput(getAverageMQ7CO());
}

/////////////////////////////////////////////