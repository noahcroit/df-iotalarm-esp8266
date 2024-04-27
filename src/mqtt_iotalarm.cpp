#include <main.h>



AsyncMqttClient mqttClient;



void onMqttConnect(bool sessionPresent) {
    debugln("MQTT connected successfully!");
    debug("Session present: ");
    debugln(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    debugln("Disconnected from MQTT.");
}

void mqtt_init (deviceConfigType *s_config) {
    // Set broker URL, port etc.
    debug("set MQTT broker to ");
    debug(s_config->mqttBrokerUrl);
    debug(", port=");
    debugln(s_config->mqttPort);
    // mqttClient.setCredentials(s_config->mqttUser, s_config->mqttPassword);
    mqttClient.setServer(s_config->mqttBrokerUrl, s_config->mqttPort);
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
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

void mqtt_sendAlarm (deviceConfigType *s_config, int type) {
    if (type == ALARM_HELP) {
        debugln("Sending HELP");
        mqttClient.publish(s_config->mqttTopicHelp, 0, false, "1");
    }
    else if (type == ALARM_SECURITY) {
        debugln("Sending SECURITY");
        mqttClient.publish(s_config->mqttTopicSecurity, 0, false, "2");
    }
}
 
void mqtt_connect (deviceConfigType *s_config) {
    // send MQTT connect to broker
    debugln("Connecting to MQTT broker");
    mqttClient.connect();
}


