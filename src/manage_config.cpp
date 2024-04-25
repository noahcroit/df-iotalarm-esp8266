#include <main.h>



WiFiManager wm; 



char* copyString(const char* src, int size) {
    char* s;
    s = (char *)malloc(size * sizeof(char));
    strcpy(s, src);
    return (char *)s;
}

bool loadConfigJSON(const char* filename, deviceConfigType *s_config) {
    if (!LittleFS.begin()) {
        debugln("Failed to mount file system");
        return false;
    }
    File configFile = LittleFS.open(filename, "r");
    if (!configFile) {
        debugln("Failed to open config file");
        return false;
    }
    JsonDocument doc;
    auto error = deserializeJson(doc, configFile);
    if (error) {
        debugln("Failed to parse config file");
        return false;
    }
    const char* configAP = doc["config_ap"];
    const char* broker = doc["mqtt_broker"];
    const char* port = doc["mqtt_port"];
    const char* help = doc["topic_help"];
    const char* security = doc["topic_security"];
    s_config->configAP = copyString(configAP, 20);
    s_config->mqttBrokerUrl = copyString(broker, 20);
    s_config->mqttPort = copyString(port, 20);
    s_config->mqttTopicHelp = copyString(help, 20);
    s_config->mqttTopicSecurity = copyString(security, 20);
    return true;
}

bool saveConfigJSON(const char* filename, deviceConfigType *s_config) {
    JsonDocument doc;
    doc["config_ap"] = s_config->configAP;
    doc["mqtt_broker"] = s_config->mqttBrokerUrl;
    doc["mqtt_port"] = s_config->mqttPort;
    doc["topic_help"] = s_config->mqttTopicHelp;
    doc["topic_security"] = s_config->mqttTopicSecurity;

    File configFile = LittleFS.open(filename, "w");
    if (!configFile) {
        debugln("Failed to open config file for writing");
        return false;
    }
    serializeJson(doc, configFile);
    return true;
}

bool resetWifiConfig(deviceConfigType *s_config) {
    char mqtt_broker[40] = "Broker URL";
    char mqtt_port[10] = "Port";
    
    WiFi.mode(WIFI_STA);
    bool res=false;
    // Set config portal timeout
    wm.setConfigPortalTimeout(300);
    // Create config menu
    WiFiManagerParameter custom_mqtt_broker("ID:MQTT_BROKER_URL", "MQTT broker URL", mqtt_broker, 40);
    WiFiManagerParameter custom_mqtt_port("ID:MQTT_BROKER_PORT", "MQTT broker port", mqtt_port, 40);
    wm.addParameter(&custom_mqtt_broker);
    wm.addParameter(&custom_mqtt_port);
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
