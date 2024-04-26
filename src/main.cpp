#include <main.h>
#include <timerScheduler.h>



// Tasks
void task_blinkStatusLED();
void task_alarmCheck();
void task_wifiManagement();
void task_mqttManagement();
void task_firmwareUpdateOTA();



// Task scheduler
TimerScheduler schd;



// Global variables
const char* configFile = "/config.json";
bool waitWifiConnection = false;
bool isFirmwareUpdateRequest = false;
deviceConfigType s_config;
deviceStateType dState;



void setup(){
    // Hardware Initialize
    s_config.ioHelp = IO_HELP; 
    s_config.ioSecurity = IO_SECURITY; 
    s_config.ioStatusLED = IO_STATUS_LED;
    s_config.ioWifiReset = IO_WIFI_RST;
    s_config.baudrate = UART_BAUD;
    s_config.mqttBrokerUrl = "mqtt-dashboard.com";
    s_config.mqttPort = "1883";
    bsp_hwInit(&s_config); 
    bsp_turnOffStatusLED(&s_config);

    // Device Configuration from JSON file
    loadConfigJSON(configFile, &s_config);

    // MQTT Initialization
    mqtt_init (&s_config);
 
    // Create Tasks
    PeriodTask t1(TASK_PERIOD_BLINKSTATUS, &task_blinkStatusLED); 
    PeriodTask t2(TASK_PERIOD_ALARMCHECK, &task_alarmCheck);
    PeriodTask t3(TASK_PERIOD_WIFIMANAGEMENT, &task_wifiManagement);
    PeriodTask t4(TASK_PERIOD_MQTTMANAGEMENT, &task_mqttManagement);
    PeriodTask t5(TASK_PERIOD_OTA, &task_firmwareUpdateOTA);
    schd.addTask(t1);
    schd.addTask(t2);
    schd.addTask(t3);
    schd.addTask(t4);
    schd.addTask(t5);
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
    debugln("********** task blink statue LED");
    if (dState.wifiState == WIFI_DISCONNECTED || dState.wifiState == WIFI_CONNECTING) {
        bsp_turnOffStatusLED(&s_config);
    }
    if(dState.wifiState == WIFI_RECONFIG) {
        bsp_turnOnStatusLED(&s_config);
    }
    if(dState.wifiState == WIFI_CONNECTED) {
        bsp_turnOnStatusLED(&s_config);
        delay(30);
        bsp_turnOffStatusLED(&s_config);
    }
}

void task_alarmCheck() {
    // SECURITY alarm signal
    if (bsp_isSecurityOccur(&s_config)) {
        dState.securityLowCnt++;
        if (dState.securityLowCnt > THRESHOLD_ALARM_LOW_COUNT) {
            // do something when SECURITY event occur
            // send MQTT alarm message here
            //
            debugln("SECURITY detected!");
            dState.securityLowCnt = 0; //reset counter
        }
    }
    // HELP alarm signal
    if (bsp_isHelpOccur(&s_config)) {
        dState.helpLowCnt++;
        if (dState.helpLowCnt > THRESHOLD_ALARM_LOW_COUNT) {
            // do something when HELP event occur
            // send MQTT alarm message here
            //
            debugln("HELP detected!");
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
                WiFi.begin();
                dState.wifiState = WIFI_CONNECTING;
                dState.wifiConnectAttemptCnt = 0;
            }
            break;

        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                debug("WiFi Connected!, IP=");
                debugln(WiFi.localIP());
                dState.wifiState = WIFI_CONNECTED;
            }
            else {
                dState.wifiConnectAttemptCnt++;
                if (dState.wifiConnectAttemptCnt >= THRESHOLD_WIFI_CONNECT_ATTEMPT) {
                    debugln("WiFi timeout");
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
            }
            break;

        case WIFI_RECONFIG:
            bool res_reset;
            debugln("WIFI Re-config...");
            res_reset = resetWifiConfig(&s_config);
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
            break;
        case MQTT_DISCONNECTED:
            break;
        case MQTT_CONNECTED:
            break;
    }
}

void task_firmwareUpdateOTA() {
    debugln("Check update firmware...");
}
