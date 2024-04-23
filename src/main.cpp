#include <main.h>
#include <timerScheduler.h>



// Tasks
void task_blinkStatusLED();
void task_alarmCheck();
void task_wifiManagement();
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
    bsp_hwInit(&s_config); 

    // Netw Configuration from JSON file
    loadConfigJSON(configFile, &s_config);
 
    // Create Tasks
    PeriodTask t1(TASK_PERIOD_BLINKSTATUS, &task_blinkStatusLED); 
    PeriodTask t2(TASK_PERIOD_ALARMCHECK, &task_alarmCheck);
    PeriodTask t3(TASK_PERIOD_WIFIMANAGEMENT, &task_wifiManagement);
    PeriodTask t4(TASK_PERIOD_OTA, &task_firmwareUpdateOTA);
    schd.addTask(t1);
    schd.addTask(t2);
    schd.addTask(t3);
    schd.addTask(t4);
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
    if (dState.wifiState == WIFI_CONNECTED) {
        bsp_toggleStatusLED(&s_config);
    }
    if(dState.wifiState == WIFI_RESET) {
        bsp_turnOnStatusLED(&s_config);
    }
}

void task_alarmCheck() {
    // SECURITY alarm signal
    if (bsp_isSecurityOccur(&s_config)) {
        dState.securityLowCnt++;
        if (dState.securityLowCnt > THRESHOLD_ALARM_LOW_COUNT) {
            // do something when SECURITY event occur
            // send MQTT alarm message, etc.
            debugln("SECURITY detected!");
            dState.securityLowCnt = 0; //reset counter
        }
    }
    // HELP alarm signal
    if (bsp_isHelpOccur(&s_config)) {
        dState.helpLowCnt++;
        if (dState.helpLowCnt > THRESHOLD_ALARM_LOW_COUNT) {
            // do something when HELP event occur
            // send MQTT alarm message, etc.
            debugln("HELP detected!");
            dState.helpLowCnt = 0; //reset counter
        }
    }
}

void task_wifiManagement() {
    // WiFi state machine
    switch (dState.wifiState) {
        case WIFI_DISCONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                debugln("Connect to previous WiFi AP...");
                WiFi.begin();
                dState.wifiState = WIFI_CONNECTING;
            }
            break;

        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                debug("WiFi Connected, IP = ");
                debugln(WiFi.localIP());
                dState.wifiState = WIFI_CONNECTED;
            }
            break;

        case WIFI_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                dState.wifiState = WIFI_DISCONNECTED;
            }
            break;

        case WIFI_RESET:
            debugln("WIFI Re-config......");
            resetWifiConfig(&s_config);
            dState.wifiState = WIFI_DISCONNECTED;
            break;
    }
    
    // check WiFi reset button
    if(bsp_isWifiResetButtonPressed(&s_config)) {
        // set period blinking task
        dState.wifiState = WIFI_RESET;
    }
}

void task_firmwareUpdateOTA() {
    debugln("Check update firmware...");
}
