# df-iotalarm-esp8266
Digital Focus's WiFi alarm gateway firmware with esp8266

### Build & Upload firmware
This repo use `platformio` framework. run this to build and upload firmware.
```
pio run
pio run --target uploadfs
pio run --target upload
```

### Configuration
The configuration of IoT alarm can be customized with `/data/config.json`.
List of param
- AP name when in WIFI reset mode
- MQTT topics
- NTP server
- OTA URL

### Test reading info from device
To read info, `mosquitto` MQTT client can be used to subscribe to data from IoT Alarm.
```
mosquitto_sub -h broker.hivemq.com -t "iotalarm/info" -t "iotalarm/help" -t "iotalarm/security"
```

### OTA Server
For testing, use python's HTTP server module to download firmware (.bin)
```
cd ~
python -m http.server 8000
```
Publish MQTT to OTA topic with chip ID to start OTA process.
```
mosquitto_pub -h broker.hivemq.com -t "iotalarm/ota" -m "<device id>"
```

