#include <main.h>
#include <timerScheduler.h>



void hwInit();

// Tasks
void task_blinkStatusLED();
void task_optoSignalCheck();
void task_WiFiManagement();
void task_firmwareUpdate();



// Task scheduler
TimerScheduler schd;



// Global variables
const char* configFile = "/config.json";
bool waitWifiConnection = false;
bool isFirmwareUpdateRequest = false;
networkConfigType s_network;
hwConfigType s_hwConfig;
 



void setup(){
    // Hardware Initialize
    s_hwConfig.ioHelp = IO_HELP; 
    s_hwConfig.ioSecurity = IO_SECURITY; 
    s_hwConfig.ioStatusLED = IO_STATUS_LED;
    s_hwConfig.ioWifiReset = IO_WIFI_RST;
    s_hwConfig.baudrate = 9600;
    bsp_hwInit(&s_hwConfig); 

    // Network Configuration from JSON file
    loadConfigJSON(configFile, &s_network);
    Serial.print("Loaded ssid: ");
    Serial.println(s_network.ssid);
    Serial.print("Loaded pwd: ");
    Serial.println(s_network.pwd);
 
    // Create Tasks
    PeriodTask t1(500, &task_blinkStatusLED);
    PeriodTask t2(200, &task_optoSignalCheck);
    PeriodTask t3(3000, &task_WiFiManagement);
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
    Serial.println("********** task blink statue LED");
    bsp_toggleStatusLED(&s_hwConfig);
}

void task_optoSignalCheck() {
    if (bsp_isSecurityOccur(&s_hwConfig)) {
        // do something when SECURITY event occur
        // send MQTT message, etc.
        Serial.println("SECURITY!");
    }
    if (bsp_isHelpOccur(&s_hwConfig)) {
        // do something when HELP event occur
        // send MQTT message, etc.
        Serial.println("HELP!");
    }
}

void task_WiFiManagement() {
    if (WiFi.status() != WL_CONNECTED) {
        if(!waitWifiConnection) {
            Serial.println("Connection WiFi...");
            WiFi.begin(s_network.ssid, s_network.pwd);
            waitWifiConnection = true;
        }
    }
    else{
        if (waitWifiConnection) {
            waitWifiConnection = false;
            Serial.println("WiFi Connected");
            Serial.println(WiFi.localIP());
        }
        else{
            Serial.println("WiFi currently Connected");
        }
    }
}

void task_firmwareUpdate() {
}
