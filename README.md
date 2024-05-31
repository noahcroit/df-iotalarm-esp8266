# df-iotalarm-esp8266
Digital Focus's WiFi alarm gateway firmware with esp8266. `platformio` with arduino-style coding is used.

### Build & Upload firmware
Use `platformio` framework can be used to build and upload. by run these
```
pio run
pio run --target uploadfs
pio run --target upload
```
These commands will build the firmware. Then, upload the config JSON file to the file system of ESP8266 first with `--target uploadfs`.
The last will be upload the firmware with '--target upload'.

### Configuration
The configuration of IoT alarm can be customized with `/data/config.json`.
List of param
- AP name when in WIFI reset mode
- MQTT topics
- NTP server
- OTA URL
- etc.

### Test reading info from device
To read info, `mosquitto` MQTT client can be used to subscribe to data from IoT Alarm.
Test MQTT broker can be `broker.hivemq.com`. It's recommended to use your own broker.
```
mosquitto_sub -h <MQTT broker URL> -t "iotalarm/info" -t "iotalarm/help" -t "iotalarm/security"
mosquitto_sub -h broker.hivemq.com -t "iotalarm/info" -t "iotalarm/help" -t "iotalarm/security"
```

### OTA Server
For testing, python's HTTP server module can be used for the firmware hosting
```
cd /path/to/this/repo
python -m http.server 8000
```

Send upgrade request to the firmware host by publishing the MQTT to OTA topic with `chip ID` to start OTA process.
```
mosquitto_pub -h <MQTT broker URL> -t "iotalarm/ota" -m "<chip ID>"
```

