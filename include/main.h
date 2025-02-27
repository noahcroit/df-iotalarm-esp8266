#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <AsyncMqttClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#define IO_FLASH        0
#define IO_HELP         2
#define IO_SECURITY     4
#define IO_WIFI_RST     5
#define IO_STATUS_LED   16
#define THRESHOLD_ALARM_LOW_COUNT 6
#define THRESHOLD_WIFI_CONNECT_ATTEMPT 10
#define THRESHOLD_MQTT_CONNECT_ATTEMPT 10
#define UART_BAUD 115200
#define TASK_PERIOD_BLINKSTATUS 1000
#define TASK_PERIOD_ALARMCHECK 500
#define TASK_PERIOD_WIFIMANAGEMENT 2000
#define TASK_PERIOD_MQTTMANAGEMENT 2000
#define TASK_PERIOD_UPDATEINFO 180000
#define TASK_PERIOD_UPDATE_TIMESTAMP 60000
#define TASK_PERIOD_OTA 3000
#define NTP_OFFSET_TH 7*60*60

typedef struct
{
    char configAP[40];
    char mqttBrokerUrl[40];
    int mqttPort;
    char mqttUser[20];
    char mqttPassword[20];
    char mqttTopicHelp[40];
    char mqttTopicSecurity[40];
    char mqttTopicInfo[40];
    char mqttTopicOta[40];
    char mqttTopicEcho[40];
    char ntpServer[20];
    char otaUrl[200];
    int8_t ioHelp;
    int8_t ioSecurity;
    int8_t ioStatusLED;
    int8_t ioWifiReset;
    int baudrate;

}deviceConfigType;

enum WifiState {
    WIFI_INIT,
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_RECONFIG
};
enum MqttState {
    MQTT_INIT,
    MQTT_DISCONNECTED,
    MQTT_CONNECTING,
    MQTT_CONNECTED
};
typedef struct
{
    int8_t wifiState = WIFI_INIT;
    int8_t mqttState = MQTT_INIT;
    int8_t wifiConnectAttemptCnt = 0;
    int8_t mqttConnectAttemptCnt = 0;
    int8_t securityLowCnt = 0;
    int8_t helpLowCnt = 0;
    bool alreadySubscribe = false;
    bool otaRequest = false;
    String ssid;
    IPAddress ip;
    int rssi;
    uint32_t chipId;
    String uid;
    String timestamp;
    unsigned long timestampEpoch;

}deviceStateType;

enum AlarmType {
    ALARM_HELP,
    ALARM_SECURITY
};

// Choose to use Serial.print() for debugging or not
#if DEBUG == 1
#define debug(x)    Serial.print(x)
#define debugln(x)  Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 * function prototypes which don't want cpp compiler to do name-mangling
 *
 *
 */
bool loadConfig (deviceConfigType *s_config);
bool loadConfigJSON (const char* filename, deviceConfigType *s_config);
bool saveConfigJSON (const char* filename, deviceConfigType *s_config);
bool resetWifiConfig (deviceConfigType *s_config, uint32_t chipId);
void firmwareUpdate(deviceConfigType *s_config);

bool bsp_hwInit (deviceConfigType *s_config); 
void bsp_toggleStatusLED (deviceConfigType *s_config);
void bsp_turnOffStatusLED (deviceConfigType *s_config);
void bsp_turnOnStatusLED (deviceConfigType *s_config);
bool bsp_isSecurityOccur (deviceConfigType *s_config);
bool bsp_isHelpOccur (deviceConfigType *s_config);
bool bsp_isWifiResetButtonPressed (deviceConfigType *s_config);

void mqtt_init (deviceConfigType *s_config, deviceStateType *s_state);
void mqtt_sendDeviceState (deviceConfigType *s_config, deviceStateType *s_state);
void mqtt_sendAlarm (deviceConfigType *s_config, deviceStateType *s_state, int type);
void mqtt_subscribeOtaRequest (deviceConfigType *s_config);
void mqtt_subscribeEcho (deviceConfigType *s_config);
void mqtt_connect (deviceConfigType *s_config);

void timestamp_ntpInit(deviceConfigType *s_config, deviceStateType *s_state);
void timestamp_start(deviceStateType *s_state);
void timestamp_updateRequest(deviceStateType *s_state);
String epoch2Date(unsigned long epoch);
#ifdef __cplusplus
}
#endif
#endif
