# Hardware

| File | Content |
|---|---|
| [`aqem-v1/AQEM-V1_schematic.pdf`](aqem-v1/AQEM-V1_schematic.pdf) | Schematic (1 sheet, A3) |
| [`aqem-v1/AQEM-V1_BOM.csv`](aqem-v1/AQEM-V1_BOM.csv) | Bill of materials |
| [`aqem-v1/AQEM-V1_ibom.html`](aqem-v1/AQEM-V1_ibom.html) | Interactive BOM for assembly (download and open in a browser) |
| [`aqem-v1/source`](aqem-v1/source) | Schematic and board as EAGLE files (Fusion Electronics 9.7) |

Fabrication data in the [release](https://github.com/marvinramm/AQEM-Air-Quality-Environment-Module/releases):

- **Gerber + Excellon drill**, generated from the EAGLE board in [`aqem-v1/source`](aqem-v1/source) with KiCad's EAGLE importer, including the slotted holes of the USB-C connector. Check them in a Gerber viewer before you order.
- **ODB++** exported from Fusion Electronics. It was exported from layout version v43, the design files are v45; the part placement is identical.

> [!WARNING]
> The Geiger tube runs at about 400 V. The converter only delivers a very small current, but touching the tube, R18 or C12/C13 while the board is powered still gives a shock. Unplug USB before you work on the board.

## Board

2 layers, 78.3 × 88.1 mm, M3 mounting holes. Milled slots separate the warm parts (LDO, ESP32-C6, HV converter) from the climate sensors, so the temperature measurement is less affected by self-heating. All parts are on the top side except the Pair button.

| Block | Parts | Function |
|---|---|---|
| USB-C | U1, D1 (SP0503BAHTG), R4/R5 | 5 V input and native USB, ESD protection |
| 3.3 V | IC2 (RT9080-33), R1 | LDO, 600 mA |
| MCU | IC1 (ESP32-C6-MINI-1-N4) | Zigbee / Thread, Wi-Fi 6, BLE, 4 MB flash |
| HV converter | IC3 (TLC555), Q1 (MMBTA44), Q2, L1, D3, C12/C13 | Boost converter for the about 400 V tube voltage |
| Geiger tube | GC1 (J305, 80 mm), R18 | Tube in clips at the board edge, 4.7 MΩ anode resistor |
| Pulse detector | Q4 (MMBT3904), C15, R19/R20 | Turns each tube pulse into a short low pulse on GPIO14 |
| Climate | U2 (BME680) | Temperature, humidity, pressure, gas resistance |
| CO2 | IC6 (SCD40) | Photoacoustic NDIR CO2 sensor |
| VOC | IC4 (SGP30) | TVOC and eCO2 |
| CO | U3 (MQ-7), R21 | Metal-oxide CO sensor, heater on 3.3 V |
| Particulate matter | PMS5003 on JP1 | PM1.0 / PM2.5 / PM10 over UART, 5 V supply |
| Sound | IC5 (MAX9814 board) | Microphone with automatic gain control, analog on GPIO2 |
| Brightness | Q3 (PT17-21C), R17 | Phototransistor, analog on GPIO4 |
| User interface | LED1 (RGB), SW1 (Pair/BOOT, bottom), SW2 (RESET) | Status LED and buttons |
| Programming | J1 (TC2030) | Tag-Connect with USB D+/D−, EN and BOOT |

## ESP32-C6 pin assignment

| GPIO | Function | | GPIO | Function |
|---|---|---|---|---|
| 0 | UART TX → PMS5003 | | 6 / 7 | I²C SDA / SCL |
| 1 | UART RX ← PMS5003 | | 8 | LED blue |
| 2 | Microphone (ADC) | | 9 | BOOT / Pair button |
| 3 | MQ-7 (ADC) | | 12 / 13 | USB D− / D+ |
| 4 | Phototransistor (ADC) | | 14 | Geiger pulses |
| 5 | LED green | | 16 | LED red |

I²C addresses: BME680 `0x76`, SCD40 `0x62`, SGP30 `0x58`.

## Lessons from building the first boards

- **BME680 orientation:** the air hole does not mark pin 1. Use the separate marking on top of the package. A rotated BME680 does not show up on the I²C bus.
- **Pulse detector:** with a wrong resistor value in the pulse detector, the transistor amplified background noise instead of the tube pulses and the CPM was far too high. If your CPM is far above the normal background (roughly 10–30 CPM for a J305), check R18–R20 against the BOM and look at GPIO14 with an oscilloscope.
- **Brightness:** inside the 3D-printed case the phototransistor is not very sensitive. A different resistor helped only a little. A next revision should move it to the edge of the board or outside the case.
- **Temperature offset:** the BME680 reads a bit high because of the heat from the neighbouring parts. More cut-outs or an offset in the firmware (measured against a reference thermometer) help.
- **Soldering:** the sensors are sensitive to heat. Reflow on a hot plate with little paste and keep the temperature under control.

## License

The hardware design files and documentation are licensed under [CC BY-NC 4.0](LICENSE). Commercial use needs my agreement, see [LICENSING.md](../LICENSING.md).
