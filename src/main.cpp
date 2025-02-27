#include <main.h>
#include <timerScheduler.h>



// Tasks
void task_blinkStatusLED();
void task_alarmCheck();
void task_wifiManagement();
void task_mqttManagement();
void task_updateInfo();
void task_firmwareUpdateOTA();
void task_updateTimestamp();



// Task scheduler
TimerScheduler schd;



// Global variables
const char* configFile = "/config.json";
bool waitWifiConnection = false;
bool isFirmwareUpdateRequest = false;
deviceConfigType s_config;
deviceStateType dState;



void setup(){
    // get chip ID 
    dState.chipId = ESP.getChipId();

    // Hardware Initialize
    s_config.ioHelp = IO_HELP; 
    s_config.ioSecurity = IO_SECURITY; 
    s_config.ioStatusLED = IO_STATUS_LED;
    s_config.ioWifiReset = IO_WIFI_RST;
    s_config.baudrate = UART_BAUD;
    bsp_hwInit(&s_config); 
    bsp_turnOffStatusLED(&s_config);
    debugln("*****");
    debug("Fimware Version=");
    debugln(FIRMWARE_VERSION);
    debug("Chip ID=");
    debugln(dState.chipId);
    debugln("*****");

    // Device Configuration from JSON file
    //loadConfigJSON(configFile, &s_config);
    loadConfig(&s_config);
    debugln("******************* Load config ********************");
    debugln(s_config.configAP);
    debugln(s_config.mqttBrokerUrl);
    debugln(s_config.mqttPort);
    debugln(s_config.mqttTopicHelp);
    debugln(s_config.mqttTopicSecurity);
    debugln(s_config.mqttTopicInfo);
    debugln(s_config.mqttTopicOta);
    debugln(s_config.ntpServer);
    debugln(s_config.otaUrl);
    debugln("****************************************************");
    
    // NTP Timestamp Initialize
    timestamp_ntpInit(&s_config, &dState);

    // Create Tasks
    PeriodTask t1(TASK_PERIOD_BLINKSTATUS, &task_blinkStatusLED); 
    
    PeriodTask t2(TASK_PERIOD_ALARMCHECK, &task_alarmCheck);
    PeriodTask t3(TASK_PERIOD_WIFIMANAGEMENT, &task_wifiManagement);
    PeriodTask t4(TASK_PERIOD_MQTTMANAGEMENT, &task_mqttManagement);
    PeriodTask t5(TASK_PERIOD_UPDATEINFO, &task_updateInfo);
    PeriodTask t6(TASK_PERIOD_OTA, &task_firmwareUpdateOTA);
    PeriodTask t7(TASK_PERIOD_UPDATE_TIMESTAMP, &task_updateTimestamp);
    delay(3000);
    
    schd.addTask(t1);
    schd.addTask(t2);
    schd.addTask(t3);
    schd.addTask(t4);
    schd.addTask(t5);
    schd.addTask(t6);
    schd.addTask(t7);
}

void loop(){
    /*
     * Run task scheduler
     * This will run all tasks and it is a blocking loop.
     * scheduler will not stop unless schd.stop() is called using Interrupt technique to yield the scheduler.
     *
     */
    schd.start();

    /*
     * Code section when scheduler stopped run. 
     *
     */
    setup();
}



void task_blinkStatusLED() {
    if(dState.wifiState == WIFI_RECONFIG) {
        bsp_turnOnStatusLED(&s_config);
    }
    else {
        if(dState.wifiState == WIFI_CONNECTED && dState.mqttState == MQTT_CONNECTED) {
            bsp_turnOnStatusLED(&s_config);
            delay(30);
            bsp_turnOffStatusLED(&s_config);
        }
        else {
            bsp_turnOffStatusLED(&s_config);
        }
    }
}

void task_alarmCheck() {
    // SECURITY alarm signal
    if (bsp_isSecurityOccur(&s_config)) {
        dState.securityLowCnt++;
        if (dState.securityLowCnt > THRESHOLD_ALARM_LOW_COUNT) {
            debugln("SECURITY detected!");
            // do something when SECURITY event occur
            // send MQTT alarm message here
            if (WiFi.status() == WL_CONNECTED && dState.mqttState == MQTT_CONNECTED) {
                mqtt_sendAlarm (&s_config, &dState, ALARM_SECURITY);
            }
            dState.securityLowCnt = 0; //reset counter
        }
    }
    // HELP alarm signal
    if (bsp_isHelpOccur(&s_config)) {
        dState.helpLowCnt++;
        if (dState.helpLowCnt > THRESHOLD_ALARM_LOW_COUNT) {
            debugln("HELP detected!");
            // do something when HELP event occur
            // send MQTT alarm message here
            if (WiFi.status() == WL_CONNECTED && dState.mqttState == MQTT_CONNECTED) {
                mqtt_sendAlarm (&s_config, &dState, ALARM_HELP);
            }
            dState.helpLowCnt = 0; //reset counter
        }
    }
}

void task_wifiManagement() {
    // WiFi management state machine
    switch (dState.wifiState) {
        case WIFI_INIT:
            debugln("WIFI param init, go to DISCONNECTED state first");
            dState.wifiState = WIFI_DISCONNECTED;
            break;

        case WIFI_DISCONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                debugln("Attemp to connect to the previous WiFi AP...");
                dState.wifiConnectAttemptCnt = 0;
                dState.wifiState = WIFI_CONNECTING;
                WiFi.begin();
            }
            break;

        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                dState.ssid = WiFi.SSID();
                dState.rssi = WiFi.RSSI();
                dState.ip = WiFi.localIP();
                debugln("WiFi Connected!");
                debug(dState.ssid);
                debug(", ");
                debugln(dState.ip);
                debug("RSSI = ");
                debugln(dState.rssi);
                dState.wifiState = WIFI_CONNECTED;
                timestamp_start(&dState);
            }
            else {
                dState.wifiConnectAttemptCnt++;
                if (dState.wifiConnectAttemptCnt >= THRESHOLD_WIFI_CONNECT_ATTEMPT) {
                    debugln("WiFi connect timeout");
                    dState.wifiState = WIFI_DISCONNECTED;
                }
            }
            break;

        case WIFI_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                dState.wifiState = WIFI_DISCONNECTED;
            }
            else{
                // Read WIFI param, RSSI etc
                dState.rssi = WiFi.RSSI();
            }
            break;

        case WIFI_RECONFIG:
            bool res_reset;
            debugln("WIFI Re-config...");
            res_reset = resetWifiConfig(&s_config, dState.chipId);
            if (res_reset) {
                debugln("Reset by itself for new WiFi config...");
                ESP.restart();
            }
            else {
                dState.wifiState = WIFI_DISCONNECTED;
            }
            break;
    }
   
    // check WiFi reset button
    if(bsp_isWifiResetButtonPressed(&s_config)) {
        // set period blinking task
        dState.wifiState = WIFI_RECONFIG;
    }
}

void task_mqttManagement() {
    // MQTT management state machine
    switch (dState.mqttState) {
        case MQTT_INIT:
            debugln("MQTT param init...");
            mqtt_init(&s_config, &dState);
            dState.mqttState = MQTT_DISCONNECTED;
            break;
        case MQTT_DISCONNECTED:
            if (WiFi.status() == WL_CONNECTED) {
                debugln("Attempt to connect to MQTT broker...");
                dState.alreadySubscribe = false;
                dState.mqttConnectAttemptCnt = 0;
                dState.mqttState = MQTT_CONNECTING;
                mqtt_connect(&s_config);
            }
            break;
        case MQTT_CONNECTING:
            dState.mqttConnectAttemptCnt++;
            if (dState.mqttConnectAttemptCnt >= THRESHOLD_MQTT_CONNECT_ATTEMPT) {
                debugln("MQTT connect timeout");
                dState.mqttState = MQTT_DISCONNECTED;
            }
            break;
        case MQTT_CONNECTED:
            if (!dState.alreadySubscribe) {
                debugln("MQTT connected successfully!");
                debugln("Start subscribe OTA topic");
                mqtt_subscribeOtaRequest(&s_config);
                mqtt_subscribeEcho(&s_config);
                dState.alreadySubscribe = true;
            }
            break;
    }
}

void task_updateInfo() {
    debugln("update info of device (SSID, IP address, RSSI etc)");
    if (dState.wifiState == WIFI_CONNECTED && dState.mqttState == MQTT_CONNECTED) {
        mqtt_sendDeviceState(&s_config, &dState);
    }
    else {
        debugln("couldn't send update, device is offline");
    }
}

void task_firmwareUpdateOTA() {
    if (dState.otaRequest) {
        // update firmware here
        firmwareUpdate(&s_config);

        // restart device
        debugln("Upgrade Firmware Successfully! Restart the device.");
        delay(2000);
        ESP.restart();
    }
}

void task_updateTimestamp() {
    timestamp_updateRequest(&dState); 
    debug("Update time from NTP sever, timestamp=");
    debugln(dState.timestamp);
}

