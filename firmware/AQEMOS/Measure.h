// AQEMOS - firmware for the AQEM Air Quality Environment Module
// Copyright (c) 2025 Marvin Ramm - PolyForm Noncommercial 1.0.0, see LICENSING.md

#ifndef MEASURE_H
#define MEASURE_H

extern bool BME680Init();
extern void MeasureBME680();

extern bool SCD40Init();
extern void MeasureSCD40();

extern bool SGP30Init();
extern void MeasureSGP30();

extern void MQ7Init();
extern void MeasureMQ7();

extern void MeasurePMS5003();

extern float Temp();
extern float Pressure();

extern void PrintAllData();

extern void MeasureNoise();

extern void IRAM_ATTR handleGeigerPulse();
extern void GeigerUpdate();

extern float getAverageBME680Temp();
extern float getAverageBME680Pressure();
extern unsigned int getAverageBME680Humidity();
extern float getAverageBME680GasResistance();
extern unsigned int getAverageSCD40CO2();
extern float getAverageSCD40Temp();
extern unsigned int getAverageSCD40Humidity();
extern unsigned int getAverageSGP30CO2();
extern unsigned int getAverageSGP30TVOC();
extern unsigned int getAverageSGP30H2();
extern unsigned int getAverageSGP30Ethanol();
extern float getAverageMQ7CO();
extern unsigned int getAveragePM1_0();
extern unsigned int getAveragePM2_5();
extern unsigned int getAveragePM10();
extern unsigned int getAverageNoise();

extern float doseRate_uSv;
extern unsigned int CPM;

#endif