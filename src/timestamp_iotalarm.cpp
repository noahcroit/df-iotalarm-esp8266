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
    struct tm *ptm;
    String currentDate;
    int currentYear;
    int currentMonth;
    int monthDay;
    if (s_state->wifiState == WIFI_CONNECTED) {
        timeClient.update();
        s_state->timestamp = timeClient.getFormattedTime();
        s_state->timestampEpoch = timeClient.getEpochTime();

        ptm = gmtime ((time_t *)&(s_state->timestampEpoch));
        currentYear = ptm->tm_year+1900;
        currentMonth = ptm->tm_mon+1;
        monthDay = ptm->tm_mday;
        currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
    }
}
