#include <main.h>



char* copyString(const char* src, int size) {
    char* s;
    s = (char *)malloc(size * sizeof(char));
    strcpy(s, src);
    return (char *)s;
}

bool loadConfigJSON(const char* filename, networkConfigType *s_network) {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
        return false;
    }
    File configFile = LittleFS.open(filename, "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }
    JsonDocument doc;
    auto error = deserializeJson(doc, configFile);
    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }
    const char* ssid = doc["ssid"];
    const char* pwd = doc["pwd"];
    s_network->ssid = copyString(ssid, 20);
    s_network->pwd = copyString(pwd, 20);
    return true;
}
