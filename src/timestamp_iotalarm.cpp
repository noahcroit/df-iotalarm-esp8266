#include <main.h>



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);



void timestamp_ntpInit(deviceConfigType *s_config, deviceStateType *s_state) {
    // set NTP pool server
    timeClient.setPoolServerName(s_config->ntpServer);
    
    // set offset for thailand TH
    timeClient.setTimeOffset(NTP_OFFSET_TH);

    // set device time to zero
    s_state->timestamp = "";
    s_state->timestampEpoch = 0;
}

void timestamp_start(deviceStateType *s_state) {
    if (s_state->wifiState == WIFI_CONNECTED) {
        timeClient.begin();
    }
}

void timestamp_updateRequest(deviceStateType *s_state) {
    if (s_state->wifiState == WIFI_CONNECTED) {
        timeClient.update();
        s_state->timestamp = timeClient.getFormattedTime();
        s_state->timestampEpoch = timeClient.getEpochTime();
    }
}
