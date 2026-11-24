// Zigbee2MQTT external converter for the AQEM (Air Quality Environment Module)
// Copyright (c) 2025 Marvin Ramm - PolyForm Noncommercial 1.0.0, see LICENSING.md
//
// AQEMOS reports the model ID "AQEM_<chip ID>", so every board has its own ID.
// Replace the placeholder below with the model ID that Zigbee2MQTT logs when
// your board joins (Settings > Devices > About > "Zigbee Model").
// Several boards can be listed in zigbeeModel.

import * as m from 'zigbee-herdsman-converters/lib/modernExtend';

export default {
    zigbeeModel: ['AQEM_XXXXXXXXXXXX'],
    model: 'AQEM',
    vendor: 'Espressif',
    description: 'AQEM air quality and environment sensor',
    extend: [
        m.deviceEndpoints({endpoints: {'1': 1, '8': 8, '9': 9, '10': 10, '11': 11, '12': 12, '13': 13, '14': 14, '15': 15, '16': 16}}),

        // Standard clusters
        m.pm25(),                                   // endpoint 1: PM2.5 (PMS5003)
        m.temperature({endpointNames: ['8']}),      // endpoint 8: mean of BME680 and SCD40
        m.humidity({endpointNames: ['8']}),
        m.illuminance({endpointNames: ['9']}),      // endpoint 9: phototransistor
        m.co2({endpointNames: ['10']}),             // endpoint 10: SCD40

        // Analog Input clusters
        m.numeric({
            name: 'pm1',
            cluster: 'genAnalogInput',
            attribute: 'presentValue',
            reporting: {min: 0, max: 600, change: 1},
            description: 'PM1.0 concentration',
            unit: 'µg/m³',
            access: 'STATE_GET',
            endpointNames: ['11'],
        }),
        m.numeric({
            name: 'pm10',
            cluster: 'genAnalogInput',
            attribute: 'presentValue',
            reporting: {min: 0, max: 600, change: 1},
            description: 'PM10 concentration',
            unit: 'µg/m³',
            access: 'STATE_GET',
            endpointNames: ['12'],
        }),
        m.numeric({
            name: 'dose',
            cluster: 'genAnalogInput',
            attribute: 'presentValue',
            reporting: {min: 0, max: 100, change: 0.1},
            description: 'Gamma dose rate (J305 Geiger tube)',
            unit: 'µSv/h',
            access: 'STATE_GET',
            endpointNames: ['13'],
        }),
        m.numeric({
            name: 'noise',
            cluster: 'genAnalogInput',
            attribute: 'presentValue',
            reporting: {min: 0, max: 60, change: 1},
            description: 'Noise level (estimate, not calibrated)',
            unit: 'dB',
            access: 'STATE_GET',
            endpointNames: ['14'],
        }),
        m.numeric({
            name: 'voc',
            cluster: 'genAnalogInput',
            attribute: 'presentValue',
            reporting: {min: 0, max: 10000, change: 1},
            description: 'Total volatile organic compounds (SGP30)',
            unit: 'ppb',
            access: 'STATE_GET',
            endpointNames: ['15'],
        }),
        m.numeric({
            name: 'co',
            cluster: 'genAnalogInput',
            attribute: 'presentValue',
            reporting: {min: 0, max: 10000, change: 1},
            description: 'Carbon monoxide (MQ-7, indicative)',
            unit: 'ppm',
            access: 'STATE_GET',
            endpointNames: ['16'],
        }),
    ],
};
