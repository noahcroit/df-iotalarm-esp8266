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
const char* ssid = "";
const char* password = "";
bool waitWifiConnection = false;
bool isFirmwareUpdateRequest = false;
 



void setup(){
    // Hardware Initialize  
    hwInit();
 
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



void hwInit(){
    // Initialize hardware
    pinMode(IO_STATUS_LED, OUTPUT);
    pinMode(IO_HELP, INPUT_PULLUP);
    pinMode(IO_SECURITY, INPUT_PULLUP);
    Serial.begin(9600);
    WiFi.begin(ssid, password);
}

void task_blinkStatusLED() {
    Serial.println("********** task blink statue LED");
    digitalWrite(IO_STATUS_LED, !digitalRead(IO_STATUS_LED));
}


void task_optoSignalCheck() {
    if (!digitalRead(IO_SECURITY)) {
        // do something when SECURITY event occur
        Serial.println("SECURITY!");
    }
    if (!digitalRead(IO_HELP)) {
        // do something when HELP event occur
        Serial.println("HELP!");
    }
}

void task_WiFiManagement() {
    if (WiFi.status() != WL_CONNECTED) {
        if(!waitWifiConnection) {
            Serial.println("Connection WiFi...");
            WiFi.begin(ssid, password);
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
