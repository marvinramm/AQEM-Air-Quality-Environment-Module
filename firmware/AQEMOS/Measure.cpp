// AQEMOS - firmware for the AQEM Air Quality Environment Module
// Copyright (c) 2025 Marvin Ramm - PolyForm Noncommercial 1.0.0, see LICENSING.md

///////////////////Includes//////////////////

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "SparkFun_SCD4x_Arduino_Library.h"  //http://librarymanager/All#SparkFun_SCD4x
#include "SparkFun_SGP30_Arduino_Library.h"  //http://librarymanager/All#SparkFun_SGP30
#include <MQUnifiedsensor.h>

#include "Measure.h"
#include "Pins.h"
#include "Settings.h"
#include "Functions.h"
#include "ESP32Zigbee.h"

Adafruit_BME680 bme(&Wire);
SCD4x SCD;
SGP30 SGP;
MQUnifiedsensor MQ7("ESP-32", 3.3, 12, MQ7AO, "MQ-7");

/////////////////////////////////////////////

//////////////////Variables//////////////////

#define NUM_MEASUREMENTS 10  // number of samples in the moving average

float BME680TempArr[NUM_MEASUREMENTS] = { 0 };
float BME680PressureArr[NUM_MEASUREMENTS] = { 0 };
unsigned int BME680HumidityArr[NUM_MEASUREMENTS] = { 0 };
float BME680GasResistanceArr[NUM_MEASUREMENTS] = { 0 };

unsigned int SCD40CO2Arr[NUM_MEASUREMENTS] = { 0 };
float SCD40TempArr[NUM_MEASUREMENTS] = { 0 };
unsigned int SCD40HumidityArr[NUM_MEASUREMENTS] = { 0 };

unsigned int SGP30CO2Arr[NUM_MEASUREMENTS] = { 0 };
unsigned int SGP30TVOCArr[NUM_MEASUREMENTS] = { 0 };
unsigned int SGP30H2Arr[NUM_MEASUREMENTS] = { 0 };
unsigned int SGP30EthanolArr[NUM_MEASUREMENTS] = { 0 };

float MQ7COArr[NUM_MEASUREMENTS] = { 0 };

unsigned int PM1_0Arr[NUM_MEASUREMENTS] = { 0 };
unsigned int PM2_5Arr[NUM_MEASUREMENTS] = { 0 };
unsigned int PM10Arr[NUM_MEASUREMENTS] = { 0 };

int indexBME = 0;
int indexSCD = 0;
int indexSGP = 0;
int indexMQ7 = 0;
int indexPMS = 0;
int indexNSE = 0;

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data PMS;

unsigned int NoiseArr[NUM_MEASUREMENTS] = { 0 };

const float CONVERSION_FACTOR = 0.005644;  // J305: µSv/h per CPM
const unsigned long LED_ON_TIME = 30;      // LED on time after a pulse in ms

volatile unsigned long lastFallTime = 0;
volatile bool pulseActive = false;
volatile unsigned long validCount = 0;
volatile bool ledTrigger = false;
volatile unsigned long lastPulseTime = 0;

unsigned int Count[AVERAGE_WINDOW + 1];  // ring buffer, one entry per second
unsigned long i = 0;
unsigned long cumulativeCount = 0;
unsigned long lastReport = 0;

unsigned long now = millis();

float doseRate_uSv = 0.0;
float totalDose_uSv = 0.0;
unsigned int CPM;

void IRAM_ATTR handleGeigerPulse() {
  unsigned long now = micros();

  if (digitalRead(G_INT) == LOW) {
    pulseActive = true;
    lastFallTime = now;
  } else if (pulseActive) {
    unsigned long pulseWidth = now - lastFallTime;
    pulseActive = false;

    if (pulseWidth >= 100 && pulseWidth <= 300) {  // valid tube pulse: 0.1-0.3 ms
      validCount++;
      cumulativeCount++;
      ledTrigger = true;         // switch the LED on
      lastPulseTime = millis();  // remember the pulse time
    }
  }
}

/////////////////////////////////////////////

////////////////////Code/////////////////////

void GeigerUpdate() {
  unsigned long now = millis();

  if (ledTrigger) {
    if (doseRate_uSv > 0.2) {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
    } else {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
    }
    if (now - lastPulseTime >= LED_ON_TIME) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, HIGH);
      ledTrigger = false;
    }
  }

  // update CPM and dose rate every 1000 ms
  if (now - lastReport >= 1000) {
    lastReport = now;

    Count[i] = validCount;
    i = (i + 1) % (AVERAGE_WINDOW + 1);

    unsigned int lastIndex = (i + 1) % (AVERAGE_WINDOW + 1);
    unsigned long diff = 0;
    if (validCount >= Count[lastIndex]) diff = validCount - Count[lastIndex];
    else diff = 0;

    float windowMinutes = AVERAGE_WINDOW / 60.0;
    CPM = diff / windowMinutes;
    doseRate_uSv = CPM * CONVERSION_FACTOR;
    totalDose_uSv = cumulativeCount / (60.0 / CONVERSION_FACTOR);
  }
}

bool BME680Init() {
  if (!bme.begin(0x76, &Wire)) {
    return false;
  } else {
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_4X);
    bme.setPressureOversampling(BME680_OS_8X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);  // 320 °C for 150 ms
    return true;
  }
}

bool SCD40Init() {
  if (SCD.begin() == false) {
    return false;
  } else {
    return true;
  }
}

bool SGP30Init() {
  if (SGP.begin() == false) {
    return false;
  }
  SGP.initAirQuality();
  return true;
}

void MQ7Init() {
  //Set math model to calculate the PPM concentration and the value of constants
  MQ7.setRegressionMethod(1);  //_PPM =  a*ratio^b
  MQ7.setA(99.042);
  MQ7.setB(-1.518);  // Configure the equation to calculate CO concentration value
  MQ7.init();
  MQ7.setRL(10);
  Serial.print("Calibrating MQ7 please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ7.update();  // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ7.calibrate(RatioMQ7CleanAir);
    Serial.print(".");
  }
  MQ7.setR0(calcR0 / 10);
  Serial.println("  done!.");
  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1)
      ;
  }
}


float calculateAverage(float *arr) {
  float sum = 0;
  for (int i = 0; i < NUM_MEASUREMENTS; i++) {
    sum += arr[i];
  }
  return sum / NUM_MEASUREMENTS;
}

unsigned int calculateAverage(unsigned int *arr) {
  unsigned int sum = 0;
  for (int i = 0; i < NUM_MEASUREMENTS; i++) {
    sum += arr[i];
  }
  return sum / NUM_MEASUREMENTS;
}

void MeasureNoise() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095;
  int sample;

  // 100 ms sampling window
  while (millis() - startMillis < 100) {
    sample = analogRead(MICAO);
    if (sample > signalMax) signalMax = sample;
    if (sample < signalMin) signalMin = sample;
  }

  unsigned int peakToPeak = signalMax - signalMin;

  // convert to volts (ESP32 ADC 0-4095, 3.3 V reference)
  float voltage = (peakToPeak * 3.3) / 4095.0;

  // convert to dB (estimate, not calibrated)
  float dB = 20.0 * log10(voltage / 0.00631);  // 0.00631 V reference (adjust when calibrating)

  // In Array speichern
  NoiseArr[indexNSE] = dB;

  indexNSE = (indexNSE + 1) % NUM_MEASUREMENTS;
}

void MeasureBME680() {
  if (bme.performReading()) {
    BME680TempArr[indexBME] = bme.temperature;
    BME680PressureArr[indexBME] = bme.pressure / 100.0;
    BME680HumidityArr[indexBME] = bme.humidity;
    BME680GasResistanceArr[indexBME] = bme.gas_resistance / 1000.0;
    indexBME = (indexBME + 1) % NUM_MEASUREMENTS;
  }
}

void MeasureSCD40() {
  if (SCD.readMeasurement()) {
    SCD40CO2Arr[indexSCD] = SCD.getCO2();
    SCD40TempArr[indexSCD] = SCD.getTemperature();
    SCD40HumidityArr[indexSCD] = SCD.getHumidity();
    indexSCD = (indexSCD + 1) % NUM_MEASUREMENTS;
  }
}

void MeasureSGP30() {
  SGP.measureAirQuality();
  SGP30CO2Arr[indexSGP] = SGP.CO2;
  SGP30TVOCArr[indexSGP] = SGP.TVOC;
  //SGP.measureRawSignals();
  //SGP30H2Arr[indexSGP] = SGP.H2;
  //SGP30EthanolArr[indexSGP] = SGP.ethanol;
  indexSGP = (indexSGP + 1) % NUM_MEASUREMENTS;
}

void MeasureMQ7() {
  MQ7.update();
  MQ7COArr[indexMQ7] = MQ7.readSensor();
  indexMQ7 = (indexMQ7 + 1) % NUM_MEASUREMENTS;
}

boolean readPMSdata(Stream *s) {
  if (!s->available()) {
    return false;
  }

  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  memcpy((void *)&PMS, (void *)buffer_u16, 30);

  if (sum != PMS.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  return true;
}

void MeasurePMS5003() {
  if (readPMSdata(&Serial1)) {
    PM1_0Arr[indexPMS] = PMS.pm10_standard;
    PM2_5Arr[indexPMS] = PMS.pm25_standard;
    PM10Arr[indexPMS] = PMS.pm100_standard;
    indexPMS = (indexPMS + 1) % NUM_MEASUREMENTS;
  }
}

float getAverageBME680Temp() {
  return calculateAverage(BME680TempArr);
}
float getAverageBME680Pressure() {
  return calculateAverage(BME680PressureArr);
}
unsigned int getAverageBME680Humidity() {
  return calculateAverage(BME680HumidityArr);
}
float getAverageBME680GasResistance() {
  return calculateAverage(BME680GasResistanceArr);
}

unsigned int getAverageSCD40CO2() {
  return calculateAverage(SCD40CO2Arr);
}
float getAverageSCD40Temp() {
  return calculateAverage(SCD40TempArr);
}
unsigned int getAverageSCD40Humidity() {
  return calculateAverage(SCD40HumidityArr);
}

unsigned int getAverageSGP30CO2() {
  return calculateAverage(SGP30CO2Arr);
}
unsigned int getAverageSGP30TVOC() {
  return calculateAverage(SGP30TVOCArr);
}
unsigned int getAverageSGP30H2() {
  return calculateAverage(SGP30H2Arr);
}
unsigned int getAverageSGP30Ethanol() {
  return calculateAverage(SGP30EthanolArr);
}

float getAverageMQ7CO() {
  return calculateAverage(MQ7COArr);
}

unsigned int getAveragePM1_0() {
  return calculateAverage(PM1_0Arr);
}
unsigned int getAveragePM2_5() {
  return calculateAverage(PM2_5Arr);
}
unsigned int getAveragePM10() {
  return calculateAverage(PM10Arr);
}

unsigned int getAverageNoise() {
  return calculateAverage(NoiseArr);
}

void PrintAllData() {
  Serial.print(">Temp [°C]:");
  Serial.println((getAverageBME680Temp() + getAverageSCD40Temp()) / 2);
  Serial.print(">Pressure [hPa]:");
  Serial.println(getAverageBME680Pressure());
  Serial.print(">Humidity [%]:");
  Serial.println((getAverageBME680Humidity() + getAverageSCD40Humidity()) / 2);
  Serial.print(">Gas Resistance [KOhm]:");
  Serial.println(getAverageBME680GasResistance());

  Serial.print(">CO2 [ppm]:");
  Serial.println(getAverageSCD40CO2());
  //Serial.print(">SCD40 Temp [°C]:"); Serial.println(getAverageSCD40Temp()-1);
  //Serial.print(">SCD40 Humidity [%]:"); Serial.println(getAverageSCD40Humidity());

  //Serial.print(">SGP30 CO2e [ppm]:"); Serial.println(getAverageSGP30CO2());
  Serial.print(">TVOC [ppb]:");
  Serial.println(getAverageSGP30TVOC());
  //Serial.print(">SGP30 H2 [ppb]:"); Serial.println(getAverageSGP30H2());
  //Serial.print(">SGP30 Ethanol [ppm]:"); Serial.println(getAverageSGP30Ethanol());

  Serial.print(">CO [ppm]:");
  Serial.println(getAverageMQ7CO());

  Serial.print(">PM1.0 [ug/m^3]:");
  Serial.println(getAveragePM1_0());
  Serial.print(">PM2.5 [ug/m^3]:");
  Serial.println(getAveragePM2_5());
  Serial.print(">PM10 [ug/m^3]:");
  Serial.println(getAveragePM10());

  Serial.print(">Noise [dB]:");
  Serial.println(getAverageNoise());

  Serial.print(">Light [lx]:");
  Serial.println(map(analogRead(LIGHT), 0, 3200, 0, 10000));

  Serial.print(">CPM [Num]:");
  Serial.println(CPM);
  Serial.print(">DoseRate [µSv/h]:");
  Serial.println(doseRate_uSv, 4);
  //Serial.print(">TotalDose [µSv]:");
  //Serial.println(totalDose_uSv, 4);
}

/////////////////////////////////////////////
