# IoT Central Hydroponics
This repository contains code for an ESP32-based monitoring solution using IoT Central. The code is based on the [IoT Central Firmware repository][2], I took the ESP8266 firmware and made some changes to run it on an ESP32. You could use an ESP8266 by taking the firmware from the original repo and just updating the .ino file.

Parts needed:
- ESP32
- DHT22 sensor (air temperature and humidity)
- DS18B20 sensor (water temperature)
- 1 10K Ohm and 1 5K Ohm resistor

The DHT22 sensor is connected to GPIO 4, the DS18B20 sensor is connected to GPIO 5.

The DTDL definition for this solution is [here][1]

[1]: ./Hydroponics.json
[2]: https://github.com/Azure/iot-central-firmware 