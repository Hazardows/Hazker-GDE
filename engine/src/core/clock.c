#include "core/clock.h"

#include "platform/platform.h"

void updateClock(hclock* clock){
    if (clock->start_time != 0) {
        clock->elapsed = platformGetAbsoluteTime() - clock->start_time;
    }
}

void startClock(hclock* clock) {
    clock->start_time = platformGetAbsoluteTime();
    clock->elapsed = 0;
}

void stopClock(hclock* clock) {
    clock->start_time = 0;
}