#include <main.h>
#include <timerScheduler.h>



void task_blinkOnBoardLED();
void task_blinkStatusLED();



// test task scheduler
TimerScheduler schd;



void setup(){
    // Initialize hardware
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(IO_STATUS_LED, OUTPUT);
    Serial.begin(9600);
    
    PeriodTask t1(250, &task_blinkOnBoardLED);
    PeriodTask t2(2000, &task_blinkStatusLED);
    schd.addTask(t1);
    schd.addTask(t2);
}

void loop(){
    /*
     * Run task scheduler
     * This is the blocking loop. scheduler will not stop unless schd.stop() is called using Interrupt
     *
     */
    schd.start();
}



void task_blinkOnBoardLED(){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void task_blinkStatusLED(){
    digitalWrite(IO_STATUS_LED, !digitalRead(IO_STATUS_LED));
}
