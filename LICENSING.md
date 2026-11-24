# Licensing

**© 2025 Marvin Ramm** – https://github.com/marvinramm/AQEM-Air-Quality-Environment-Module

The AQEM is source-available for **non-commercial use**. Private persons may build it, use it, develop it further and share it, as long as they credit the author. If you earn money with the AQEM or a design based on it, you need my agreement first (see [Commercial use](#commercial-use)).

| Part | License | File |
|---|---|---|
| Firmware (`firmware/`, including the Zigbee2MQTT converter and the firmware binaries in the releases) | [PolyForm Noncommercial 1.0.0](https://polyformproject.org/licenses/noncommercial/1.0.0) | [`LICENSE`](LICENSE) |
| Hardware design files, images and documentation (`hardware/`, `docs/`, README, release files) | [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) | [`hardware/LICENSE`](hardware/LICENSE) |

Note: these licenses are not "open source" in the OSI sense, because they exclude commercial use.

## Attribution (required)

Credit the author wherever you use or share the AQEM or parts of it, e.g. in your README, in your documentation or on the PCB silkscreen. Please use this text:

> Based on "AQEM – Air Quality Environment Module" by Marvin Ramm – https://github.com/marvinramm/AQEM-Air-Quality-Environment-Module – licensed under CC BY-NC 4.0 / PolyForm Noncommercial 1.0.0.

Also state whether you made changes. For the firmware, keep the `Required Notice` line from [`LICENSE`](LICENSE) in every copy.

## What counts as non-commercial

Allowed without asking:

- building one or more AQEMs for yourself, your home or your hobby projects
- developing the design or the firmware further and publishing your version under the same terms, with attribution
- use in schools, universities, non-profit organisations and for personal research

Needs a commercial license:

- selling assembled boards, kits, PCBs or devices that contain the AQEM design or firmware
- using the design or firmware in a product or a paid service
- any other use that earns money with the AQEM or a derived design

## Commercial use

Commercial use is possible by agreement, with a share of the revenue or profit. Contact me before you start:

**marvin@ramm-eng.de**

## Third-party components

The firmware uses the ESP32 Arduino core and its Zigbee library (Espressif, Apache 2.0), the Adafruit BME680 and Unified Sensor libraries (BSD / Apache 2.0), the SparkFun SCD4x and SGP30 libraries (MIT) and MQSensorsLib (MIT). They are not part of this repository and keep their own licenses.

The particulate matter sensor (Plantower PMS5003), the MAX9814 microphone board and the J305 Geiger tube are third-party parts. Their names and trademarks belong to their owners.
