#include <main.h>



AsyncMqttClient mqttClient;
int8_t *pMqttState;
bool *pOtaRequest;
uint32_t *pChipId;
char *pTopicOta;
char *pTopicEcho;



void onMqttConnect(bool sessionPresent) {
    *pMqttState = MQTT_CONNECTED; 
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    debugln("Disconnected from MQTT.");
    *pMqttState = MQTT_DISCONNECTED; 
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    debugln("Publish received.");
    debug("  topic: ");
    debugln(topic);
    debug("  qos: ");
    debugln(properties.qos);
    debug("  dup: ");
    debugln(properties.dup);
    debug("  retain: ");
    debugln(properties.retain);
    debug("  len: ");
    debugln(len);
    debug("  index: ");
    debugln(index);
    debug("  total: ");
    debugln(total);
    debug("  payload: ");
    debugln(payload);

    // if payload is its Chip ID
    // set OTA Request flag to true, via pointer
    uint32_t id = atoi(payload);
    if (id == *pChipId) {
        debugln("Chip ID matched!");
        if (strcmp(topic, pTopicOta) == 0) {
            debugln("Set OTA request flag");
            *pOtaRequest = true;
        }
        else if (strcmp(topic, pTopicEcho) == 0) {
            debugln("Send message back to echo topic");
            mqttClient.publish(pTopicEcho, 0, false, "ECHO OK");
        }
    }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
    debugln("Subscribe acknowledged.");
    debug("  packetId: ");
    debugln(packetId);
    debug("  qos: ");
    debugln(qos);
}

void mqtt_init (deviceConfigType *s_config, deviceStateType *s_state) {
    // Set broker URL, port etc.
    debug("MQTT Init, set MQTT broker to ");
    debug(s_config->mqttBrokerUrl);
    debug(", port=");
    debugln(s_config->mqttPort);
    pTopicOta = (char *)&(s_config->mqttTopicOta);
    pTopicEcho = (char *)&(s_config->mqttTopicEcho);
    pMqttState = &(s_state->mqttState);
    pOtaRequest = &(s_state->otaRequest);
    pChipId = &(s_state->chipId);
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onSubscribe(onMqttSubscribe);
    // mqttClient.setCredentials(s_config->mqttUser, s_config->mqttPassword);
    mqttClient.setServer(s_config->mqttBrokerUrl, s_config->mqttPort);
}

void mqtt_sendDeviceState (deviceConfigType *s_config, deviceStateType *s_state) {
    JsonDocument doc;
    char payload[200];
    String str_ip;
    if (s_state->wifiState == WIFI_CONNECTED) {
        // Apply Json parse to device's state
        doc["device_id"] = s_state->chipId;
        doc["FW"] = FIRMWARE_VERSION;
        doc["ssid"] = s_state->ssid;
        doc["rssi"] = s_state->rssi;
        str_ip = s_state->ip.toString();
        doc["ip"] = str_ip;
        doc["timestamp"] = s_state->timestamp;
        serializeJson(doc, payload, sizeof(payload));
        
        // MQTT publish
        mqttClient.publish(s_config->mqttTopicInfo, 0, false, payload);
    }
}

void mqtt_sendAlarm (deviceConfigType *s_config, deviceStateType *s_state, int type) {
    JsonDocument doc;
    char payload[100];
    doc["device_id"] = s_state->chipId;
    doc["timestamp"] = s_state->timestamp;
    serializeJson(doc, payload, sizeof(payload));
    if (type == ALARM_HELP) {
        debugln("Sending HELP");
        mqttClient.publish(s_config->mqttTopicHelp, 0, false, payload);
    }
    else if (type == ALARM_SECURITY) {
        debugln("Sending SECURITY");
        mqttClient.publish(s_config->mqttTopicSecurity, 0, false, payload);
    }
}

void mqtt_subscribeOtaRequest (deviceConfigType *s_config) {
    mqttClient.subscribe(s_config->mqttTopicOta, 0);
}

void mqtt_subscribeEcho (deviceConfigType *s_config) {
    mqttClient.subscribe(s_config->mqttTopicEcho, 0);
}

void mqtt_connect (deviceConfigType *s_config) {
    // send MQTT connect to broker
    debugln("Connecting to MQTT broker");
    mqttClient.connect();
}


