# esp32_telemetry

A simple ESP32 sketch to read temperature and humidity from a DHT22 sensor, synchronize time via NTP, and publish telemetry to an MQTT broker. Supports OTA updates via ArduinoOTA.

## Hardware
- ESP32 board
- DHT22 sensor connected to GPIO4

## Features
- Wi-Fi connection
- NTP time synchronization
- MQTT publishing (topic `homelab/esp32/metrics`)
- Over-the-air updates via ArduinoOTA

## Setup
1. Install required Arduino libraries:
   - `WiFi`
   - `PubSubClient`
   - `DHT sensor library`
   - `ArduinoOTA`
2. Copy and configure [temp_sketch](temp_sketch):
   - Set your Wi-Fi credentials: `ssid`, `password`
   - Set your MQTT broker: `mqtt_server`, `mqtt_port`
   - Adjust `location` to identify this device
3. Upload the sketch to your ESP32.

## Usage
- Monitor output at 115200 baud in the Serial Monitor.
- Subscribe to the MQTT topic:
  ```sh
  mosquitto_sub -h <mqtt_server> -t homelab/esp32/metrics
  ```