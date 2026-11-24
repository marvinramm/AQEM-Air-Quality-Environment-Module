/////////////////////////////////////////////
//         AQEMOS by Marvin Ramm           //
//               REV: V0.1.8               //
//  github.com/marvinramm/AQEM-Air-        //
//     Quality-Environment-Module          //
//                                         //
//   Copyright (c) 2025 Marvin Ramm        //
//   PolyForm Noncommercial 1.0.0          //
//   Commercial use: see LICENSING.md      //
/////////////////////////////////////////////

/*
   Arduino ESP32-C6 Settings:
   Board: "ESP32C6 Dev Module"
   Upload Speed: "921600"
   CPU Frequency: "160MHz (WiFi)"
   Flash Frequency: "80MHz"
   Flash Mode: "QIO"
   Flash Size: "4MB"
   Partition Scheme: "Zigbee 4MB with spiffs"
   Zigbee Mode: "Zigbee ED (end device)"
*/


///////////////////Includes//////////////////

#include "Measure.h"
#include "Pins.h"
#include "Settings.h"
#include "Functions.h"
#include "ESP32Zigbee.h"

//////////////////Variables//////////////////

unsigned long PreviousMillis1;
unsigned long PreviousMillis2;
unsigned long PreviousMillis3;

/////////////////////////////////////////////

////////////////////Setup////////////////////

void setup() {
  HardwareInit();
}

/////////////////////////////////////////////

////////////////////Loop/////////////////////

void loop() {
  if (millis() - PreviousMillis1 >= 1000) {
    PreviousMillis1 = millis();

    MeasureSGP30();
    MeasureMQ7();
    MeasureNoise();

    PrintAllData();
  }

  if (millis() - PreviousMillis2 >= 2000) {
    PreviousMillis2 = millis();

    MeasureBME680();
    MeasureSCD40();
    MeasurePMS5003();

  }

  if (millis() - PreviousMillis3 >= 10000) {
    PreviousMillis3 = millis();

    if (EN_ZIGBEE == 1) {
      UpdateEndpoint();
    }
  }


  GeigerUpdate();
  if (EN_ZIGBEE == 1) {
    ReadPairButton();
  }
}

/////////////////////////////////////////////