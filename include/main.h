#ifndef _MAIN_H
#define _MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#define IO_FLASH        0
#define IO_HELP         2
#define IO_SECURITY     4
#define IO_WIFI_RST     5
#define IO_STATUS_LED   16

typedef struct
{
    char *ssid;
    char *pwd;
    char *mqttBrokerUrl;
    int mqttPort;

}networkConfigType;

typedef struct
{
    int8_t ioHelp;
    int8_t ioSecurity;
    int8_t ioStatusLED;
    int8_t ioWifiReset;
    int baudrate;

}hwConfigType;

#ifdef __cplusplus
extern "C" {
#endif
/*
 * function prototypes which don't want cpp compiler to do name-mangling
 *
 *
 */
char* copyString(const char* src, int size);
bool loadConfigJSON(const char* filename, networkConfigType *s_network);

bool bsp_hwInit(hwConfigType *s_config); 
void bsp_toggleStatusLED(hwConfigType *s_config);
bool bsp_isSecurityOccur(hwConfigType *s_config);
bool bsp_isHelpOccur(hwConfigType *s_config);
bool bsp_isWifiResetButtonPressed(hwConfigType *s_config);
#ifdef __cplusplus
}
#endif
#endif
