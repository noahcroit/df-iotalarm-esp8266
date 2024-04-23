#ifndef _TIMER_SCHEDULER
#define _TIMER_SCHEDULER

#define MAX_TASK_NUM 10
#define USE_ARDUINO_MILLIS  1

#ifdef USE_ARDUINO_MILLIS
#include <Arduino.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Put non-name-mangling function prototypes in here
 *
 */
class PeriodTask {
private:
    void (*cb)();

public:
    bool isReady;
    unsigned long previousTick;
    unsigned int period;
    PeriodTask(unsigned int period_ms, void (*callback)());
    PeriodTask();
    void setPeriod(unsigned int period_ms);
    void setCallback(void (*callback)());
    void run();
};

class TimerScheduler {
private:
    bool isrun;
    unsigned long currentTick;
    int numberOfTasks;
public:
    PeriodTask taskArr[MAX_TASK_NUM];
    TimerScheduler();
    void addTask(PeriodTask task);
    bool isTaskReady(PeriodTask task);
    int getNumberOfTasks();
    void start();
    void stop();
};

#ifdef __cplusplus
}
#endif

#endif
