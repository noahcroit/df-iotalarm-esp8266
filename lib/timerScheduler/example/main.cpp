#include <main.h>
#include <timerScheduler.h>



void taskPrint();
void taskBlink();



// test task scheduler
TimerScheduler schd;



void setup(){
    // Initialize hardware
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    
    PeriodTask t1(500, &taskBlink);
    PeriodTask t2(3000, &taskPrint);
    schd.addTask(t1);
    schd.addTask(t2);
}

void loop(){
    schd.start();
}



void taskPrint(){
    Serial.println("task print");
    delay(10);
}

void taskBlink(){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.println("task blinky");
    delay(10);
}
