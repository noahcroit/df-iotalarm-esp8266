#include "timerScheduler.h"



/*
 *
 * PeriodTask Class
 *
 */
PeriodTask::PeriodTask(unsigned int period_ms, void (*callback)()) {
    previousTick = 0;
    period = period_ms;
    cb = callback;
    isReady = false;
}

PeriodTask::PeriodTask() {
    previousTick = 0;
    period = 0;
    cb = nullptr;
    isReady = false;
}

void PeriodTask::setPeriod(unsigned int period_ms) {
    period = period_ms;
    previousTick = 0;
    isReady = false;
}

void PeriodTask::setCallback(void (*callback)()) {
    cb = callback;
    previousTick = 0;
    isReady = false;
}

void PeriodTask::run() {
    (*cb)();
}



/*
 *
 * TimerScheduler Class
 *
 */

TimerScheduler::TimerScheduler() {
   numberOfTasks = 0;
}

void TimerScheduler::addTask(PeriodTask task) {
    if (numberOfTasks > MAX_TASK_NUM){
        return;
    }
    taskArr[numberOfTasks] = task;
    numberOfTasks++;
}

bool TimerScheduler::isTaskReady(PeriodTask task) {
    if(currentTick - task.previousTick > task.period) {
        return true;
    }
    else{
        return false;
    }
}

int TimerScheduler::getNumberOfTasks() {
    return numberOfTasks;
}

void TimerScheduler::start() {
    isrun = true;
    while (isrun){
        /*
         * Simple Task Scheduling with Timer millis()
         * Check if Task's Tick has already reached the period
         * Warning, overflow of timer can be a problem.
         */
        for (int i=0; i<numberOfTasks; i++){
            if(isTaskReady(taskArr[i])){
                taskArr[i].run();
                taskArr[i].previousTick = currentTick;
            }
        }

        /*
         * Update currentTick
         *
         */
#ifdef USE_ARDUINO_MILLIS
        delay(1);
        currentTick = millis();
#endif
    }
}

void TimerScheduler::stop() {
    isrun = false;
}
