#include <main.h>
#include <timerScheduler.h>



// Tasks
void task_blinkStatusLED();
void task_optoSignalCheck();
void task_wifiManagement();
void task_firmwareUpdate();



// Task scheduler
TimerScheduler schd;



// Global variables
const char* configFile = "/config.json";
bool waitWifiConnection = false;
bool isFirmwareUpdateRequest = false;
networkConfigType s_network;
hwConfigType s_hwConfig;
wifiStateType wifiState;
 



void setup(){
    // Hardware Initialize
    s_hwConfig.ioHelp = IO_HELP; 
    s_hwConfig.ioSecurity = IO_SECURITY; 
    s_hwConfig.ioStatusLED = IO_STATUS_LED;
    s_hwConfig.ioWifiReset = IO_WIFI_RST;
    s_hwConfig.baudrate = 115200;
    bsp_hwInit(&s_hwConfig); 

    // Network Configuration from JSON file
    loadConfigJSON(configFile, &s_network);
    debugln("Loaded ssid: ");
    debugln(s_network.ssid);
    debugln("Loaded pwd: ");
    debugln(s_network.pwd);
 
    // Create Tasks
    PeriodTask t1(500, &task_blinkStatusLED);
    PeriodTask t2(200, &task_optoSignalCheck);
    PeriodTask t3(3000, &task_wifiManagement);
    schd.addTask(t1);
    schd.addTask(t2);
    schd.addTask(t3);
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
    bsp_toggleStatusLED(&s_hwConfig);
}

void task_optoSignalCheck() {
    if (bsp_isSecurityOccur(&s_hwConfig)) {
        // do something when SECURITY event occur
        // send MQTT message, etc.
        debugln("SECURITY detected!");
    }
    if (bsp_isHelpOccur(&s_hwConfig)) {
        // do something when HELP event occur
        // send MQTT message, etc.
        debugln("HELP detected!");
    }
}

void task_wifiManagement() {
    // WiFi state machine
    switch (wifiState.currentState) {
        case WIFI_DISCONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                debugln("Connection WiFi...");
                WiFi.begin(s_network.ssid, s_network.pwd);
                wifiState.currentState = WIFI_CONNECTING;
            }
            break;

        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                debug("WiFi Connected, IP = ");
                debugln(WiFi.localIP());
                wifiState.currentState = WIFI_CONNECTED;
            }
            break;

        case WIFI_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                wifiState.currentState = WIFI_DISCONNECTED;
            }
            break;

        case WIFI_RESET:
            //do WIFI reset with smartConfig.h
            delay(5000);
            wifiState.currentState = WIFI_DISCONNECTED;
            break;
    }
    
    // check WiFi reset button
    if(bsp_isWifiResetButtonPressed(&s_hwConfig)) {
        wifiState.currentState = WIFI_RESET;
    }
}

void task_firmwareUpdate() {
}
