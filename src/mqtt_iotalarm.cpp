#include <main.h>



AsyncMqttClient mqttClient;



void mqtt_init (deviceConfigType *s_config) {
    // Set broker URL, port etc.
    // mqttClient.setCredentials(s_config->mqttUser, s_config->mqttPassword);
    mqttClient.setServer(s_config->mqttBrokerUrl, atoi(s_config->mqttPort));
    debug("set MQTT broker to ");
    debug(s_config->mqttBrokerUrl);
    debug(", port=");
    debugln(s_config->mqttPort);
}

void mqtt_sendDeviceState (deviceConfigType *s_config, deviceStateType *s_state) {
    if (s_state->wifiState == WIFI_CONNECTED) {
        // Apply Json parse to device's state
        JsonDocument doc;
        /*
        doc["id"] = deviceId;
	    doc["ip"] = WiFi.localIP().toString();
	    doc["rssi"] = WiFi.RSSI();
	    doc["ssid"] = WiFi.SSID();
	    doc["v"] = FIRMWARE_VERSION;
        doc["status"] = true;
        String json = String();
        serializeJson(doc, json);
        */
        // MQTT connect
        //
        // MQTT publish
        //
    }
}

void mqtt_sendAlarm (deviceConfigType *s_config, deviceStateType *s_state, int type) {
    if (s_state->wifiState == WIFI_CONNECTED) {
        // MQTT connect
        //
        // MQTT publish
        //
    }
}
 
void mqtt_connect (deviceConfigType *s_config, deviceStateType *s_state) {
    if (s_state->wifiState == WIFI_CONNECTED) {
        // send MQTT connect to broker
        debugln("Connecting to MQTT broker");
        mqttClient.connect();
    }
}

