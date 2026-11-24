# AQEMOS – AQEM firmware

Firmware V0.1.8 for the ESP32-C6 on the AQEM board, written with the ESP32 Arduino core.

| File | Content |
|---|---|
| `AQEMOS.ino` | Main loop and timing |
| `Functions.cpp` | Hardware init, sensor start-up, PMS5003 UART |
| `Measure.cpp` | All sensors, moving averages, Geiger pulse counting, serial output |
| `ESP32Zigbee.cpp` | Zigbee end device with ten endpoints, pair button |
| `Pins.h` / `Settings.h` | Pin assignment and settings |
| [`zigbee2mqtt/AQEM.js`](zigbee2mqtt/AQEM.js) | External converter for Zigbee2MQTT |

## Build and flash

Requirements:

- [ESP32 Arduino core](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) **3.3.x** (tested with 3.3.3; the analog Zigbee endpoints need 3.3 or newer)
- Libraries: Adafruit BME680, Adafruit Unified Sensor, SparkFun SCD4x, SparkFun SGP30, MQUnifiedsensor (MQSensorsLib)

Arduino IDE settings:

| Setting | Value |
|---|---|
| Board | ESP32C6 Dev Module |
| Flash size | 4 MB |
| Partition scheme | Zigbee 4MB with spiffs |
| Zigbee mode | Zigbee ED (end device) |

With `arduino-cli`:

```bash
arduino-cli compile --upload -p /dev/ttyACM0 \
  --fqbn esp32:esp32:esp32c6:PartitionScheme=zigbee,ZigbeeMode=ed \
  firmware/AQEMOS
```

The USB-C port is connected to the native USB of the ESP32-C6, so no USB-serial adapter is needed. The Tag-Connect footprint J1 (TC2030) carries the same USB lines plus EN and BOOT. If the upload does not start, hold the Pair button (BOOT), press RESET and release Pair.

Precompiled binaries are attached to the [release](https://github.com/marvinramm/AQEM-Air-Quality-Environment-Module/releases):

```bash
# complete image (bootloader, partitions, app) - erases the Zigbee pairing
esptool.py --chip esp32c6 -p /dev/ttyACM0 write_flash 0x0 AQEMOS_V0.1.8_ESP32-C6_merged.bin
# app only - keeps the Zigbee pairing
esptool.py --chip esp32c6 -p /dev/ttyACM0 write_flash 0x10000 AQEMOS_V0.1.8_ESP32-C6_app.bin
```

## Measurement cycle

| Interval | What happens |
|---|---|
| 1 s | SGP30 (TVOC), MQ-7 (CO), sound level (100 ms window), all values printed on the serial port |
| 2 s | BME680, SCD40, PMS5003 |
| 10 s | All values sent to the Zigbee endpoints |
| continuous | Geiger pulses by interrupt, dose rate updated every second |

Every value is a moving average over the last 10 samples. Temperature and humidity are the mean of the BME680 and the SCD40.

The serial output (115200 baud) uses the `>name:value` format, so it can be plotted directly with [Teleplot](https://github.com/nesnes/teleplot).

### Geiger counter

The J305 tube pulses are detected on GPIO14 (both edges). Only pulses between 0.1 and 0.3 ms count, which filters out noise. The counts per minute (CPM) are averaged over a sliding window of `AVERAGE_WINDOW` seconds (default 600 s) and converted with 0.005644 µSv/h per CPM (J305). Each pulse flashes the LED green, or yellow when the dose rate is above 0.2 µSv/h.

### CO sensor

At start-up the firmware calibrates the MQ-7 (R0) for 10 samples, assuming clean air (`RatioMQ7CleanAir` = 27.5). **Start the AQEM in fresh air.** If the sensor is open or shorted, the calibration fails and the firmware stops with a message on the serial port.

## Settings (`Settings.h`)

| Define | Default | Meaning |
|---|---|---|
| `EN_ZIGBEE` | 1 | Zigbee on/off. With 0 the AQEM only prints the values on the serial port. |
| `AVERAGE_WINDOW` | 600 | Averaging window of the Geiger counter in s |
| `RatioMQ7CleanAir` | 27.5 | RS/R0 of the MQ-7 in clean air |
| `SEALEVELPRESSURE_HPA` | 1013.25 | Reference pressure |

## Zigbee

The AQEM joins an existing Zigbee network as an end device. At start-up the LED lights blue, then blinks red until the AQEM has joined the network.

**Pair button** (BOOT, GPIO9, on the bottom side): a short press sends all values immediately, holding it for 3 s resets the Zigbee settings so the AQEM can be paired again.

| Endpoint | Cluster | Value | Source |
|---|---|---|---|
| 1 | PM2.5 measurement | PM2.5 in µg/m³ | PMS5003 |
| 8 | Temperature + humidity | °C, % RH | BME680 + SCD40 |
| 9 | Illuminance | lx (approximate) | Phototransistor |
| 10 | CO2 concentration | ppm | SCD40 |
| 11 | Analog Input | PM1.0 in µg/m³ | PMS5003 |
| 12 | Analog Input | PM10 in µg/m³ | PMS5003 |
| 13 | Analog Input | Dose rate in µSv/h | J305 |
| 14 | Analog Input | Sound level in dB (estimate) | MAX9814 |
| 15 | Analog Input | TVOC in ppb | SGP30 |
| 16 | Analog Input | CO in ppm (indicative) | MQ-7 |

The model ID is `AQEM_<chip ID>`, so every board reports its own ID.

### Zigbee2MQTT and Home Assistant

1. Copy [`zigbee2mqtt/AQEM.js`](zigbee2mqtt/AQEM.js) into the `external_converters` folder of your Zigbee2MQTT data directory.
2. Pair the AQEM. Zigbee2MQTT shows its model ID (`AQEM_…`) under *About*.
3. Replace `AQEM_XXXXXXXXXXXX` in `AQEM.js` with that ID (add several IDs for several boards) and restart Zigbee2MQTT.

All values then appear as sensors in Home Assistant through MQTT discovery.

## Known limitations of V0.1.8

- The sound level and the brightness are not calibrated. dB is estimated from the peak-to-peak voltage of the MAX9814 (reference 6.31 mV), lux is a linear map of the ADC value.
- The MQ-7 is heated continuously instead of with the 5 V / 1.4 V cycle from its datasheet. The CO value is an indication, not a calibrated measurement.
- The SGP30 runs without humidity compensation, and its baseline is not saved, so it needs some time to settle after every start.
- `ZigbeeInit()` waits until the AQEM has joined a network. Without a Zigbee network, set `EN_ZIGBEE` to 0.

## Changes in this repository

Compared to the original V0.1.8 sources: comments translated to English, the board settings in the header corrected (end device mode, 160 MHz), TVOC printed in ppb (the SGP30 unit), and `SGP.initAirQuality()` is now called at start-up (before, the call came after `return` and never ran).

## License

[PolyForm Noncommercial 1.0.0](../LICENSE) – see [LICENSING.md](../LICENSING.md). Commercial use by agreement.
