#include <main.h>
#include <time.h>



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
        s_state->timestampEpoch = timeClient.getEpochTime();
        s_state->timestamp = epoch2Date(s_state->timestampEpoch);
    }
}

String epoch2Date(unsigned long epoch) {
    time_t rawtime;
    struct tm  ts;
    char       buf[80];

	rawtime=epoch;

    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&rawtime);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    return String(buf);
}
