#include <main.h>



WiFiManager wm; 



char* copyString(const char* src, int size) {
    char* s;
    s = (char *)malloc(size * sizeof(char));
    strcpy(s, src);
    return (char *)s;
}

bool loadConfigJSON(const char* filename, networkConfigType *s_network) {
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
    const char* ssid = doc["ssid"];
    const char* pwd = doc["pwd"];
    s_network->ssid = copyString(ssid, 20);
    s_network->pwd = copyString(pwd, 20);
    return true;
}

bool saveConfigJSON(const char* filename, networkConfigType *s_network) {
    JsonDocument doc;
    doc["ssid"] = s_network->ssid;
    doc["pwd"] = s_network->pwd;

    File configFile = LittleFS.open(filename, "w");
    if (!configFile) {
        debugln("Failed to open config file for writing");
        return false;
    }
    serializeJson(doc, configFile);
    return true;
}

void resetWifiConfig(networkConfigType *s_network) {
    WiFi.mode(WIFI_STA);
    //wm.resetSettings();
    bool res;
    wm.setConfigPortalTimeout(180);
    wm.startConfigPortal("DF_IoTAlarm_AP");
    //res = wm.autoConnect("DF_IoTAlarmConfig","digitalfocus"); // password protected ap
    /*
    if(!res) {
        debugln("Failed to connect");
        // ESP.restart();
    }
    else {
        //if you get here you have connected to the WiFi
        debugln("WIFI connected!");
    }
    */
    //s_networkConfig->ssid = (char *)"new ssid";
    //s_networkConfig->pwd = (char *)"new password";
}
