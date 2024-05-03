#include <main.h>



WiFiManager wm;
WiFiClient wc;



bool loadConfigJSON (const char* filename, deviceConfigType *s_config) {
    if (!LittleFS.begin()) {
        debugln("Failed to mount file system");
        return false;
    }
    File configFile = LittleFS.open(filename, "r");
    if (!configFile) {
        debugln("Failed to open config file");
        return false;
    }
    
    JsonDocument jsonDoc;
    auto error = deserializeJson(jsonDoc, configFile);
    if (error) {
        debugln("Failed to parse config file");
        return false;
    }
    
    const char *configAP = jsonDoc["config_ap"];
    const char *mqttBrokerUrl = jsonDoc["mqtt_broker"];
    int mqttPort = jsonDoc["mqtt_port"];
    const char *mqttTopicHelp = jsonDoc["topic_help"];
    const char *mqttTopicSecurity = jsonDoc["topic_security"];
    const char *mqttTopicInfo = jsonDoc["topic_info"];
    const char *mqttTopicOta = jsonDoc["topic_ota"];
    const char *ntpServer = jsonDoc["ntp_server"];
    const char *otaUrl = jsonDoc["ota_url"];
    
    strcpy(s_config->configAP, configAP);
    s_config->mqttPort = mqttPort;
    strcpy(s_config->mqttBrokerUrl, mqttBrokerUrl);
    strcpy(s_config->mqttTopicHelp, mqttTopicHelp);
    strcpy(s_config->mqttTopicSecurity, mqttTopicSecurity);
    strcpy(s_config->mqttTopicInfo, mqttTopicInfo);
    strcpy(s_config->mqttTopicOta, mqttTopicOta);
    strcpy(s_config->ntpServer, ntpServer);
    strcpy(s_config->otaUrl, otaUrl);
    return true;
}

bool saveConfigJSON (const char* filename, deviceConfigType *s_config) {
    JsonDocument jsonDoc;
    /*
    jsonDoc["config_ap"] = s_config->configAP;
    jsonDoc["mqtt_broker"] = s_config->mqttBrokerUrl;
    jsonDoc["mqtt_port"] = String(s_config->mqttPort);
    jsonDoc["topic_help"] = s_config->mqttTopicHelp;
    jsonDoc["topic_security"] = s_config->mqttTopicSecurity;
    */
    File configFile = LittleFS.open(filename, "w");
    if (!configFile) {
        debugln("Failed to open config file for writing");
        return false;
    }
    serializeJson(jsonDoc, configFile);
    return true;
}

bool resetWifiConfig (deviceConfigType *s_config) {
    char mqtt_broker[40];
    char mqtt_port[10];
    char ota_url[200];
    strcpy(mqtt_broker, s_config->mqttBrokerUrl);
    sprintf(mqtt_port, "%d", s_config->mqttPort);
    strcpy(ota_url, s_config->otaUrl);
    
    WiFi.mode(WIFI_STA);
    bool res=false;
    // Set config portal timeout
    wm.setConfigPortalTimeout(300);
    // Create config menu
    WiFiManagerParameter custom_mqtt_broker("ID:MQTT_BROKER_URL", "MQTT broker URL", mqtt_broker, 40);
    WiFiManagerParameter custom_mqtt_port("ID:MQTT_BROKER_PORT", "MQTT broker port", mqtt_port, 40);
    WiFiManagerParameter custom_ota_url("ID:OTA_URL", "OTA URL", ota_url, 120);
    wm.addParameter(&custom_mqtt_broker);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_ota_url);
    // Start portal
    res = wm.startConfigPortal(s_config->configAP);
    if(!res) {
        debugln("Failed to connect or Abort config.");
    }
    else {
        debugln("Config successfully, WIFI connected.");
        debugln("Param list:");
        strcpy(mqtt_broker, custom_mqtt_broker.getValue());
        strcpy(mqtt_port, custom_mqtt_port.getValue());
        debug("MQTT broker URL : ");
        debugln(mqtt_broker);
        debug("MQTT port : ");
        debugln(mqtt_port);
    }
    return res;
}

void firmwareUpdate(deviceConfigType *s_config) {
    debugln("Start firmware upgrade with OTA");
    ESPhttpUpdate.setClientTimeout(8000);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    t_httpUpdate_return ret = ESPhttpUpdate.update(wc, s_config->otaUrl);
    if (ret == HTTP_UPDATE_OK) {
        switch (ret) {
            case HTTP_UPDATE_FAILED:
                debugln("HTTP_UPDATE_FAILED Error");
                break;
            case HTTP_UPDATE_NO_UPDATES:
                debugln("HTTP_UPDATE_NO_UPDATES");
                break;
            case HTTP_UPDATE_OK:
                debugln("HTTP_UPDATE_OK");
                break;
        }
    }
    else{
        debug("Update is not OK, ret=");
        debugln(ret);
    }
}


