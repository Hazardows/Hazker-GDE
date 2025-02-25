#pragma once

#ifdef __cplusplus 
extern "C" { 
#endif

#include "defines.h"

typedef struct hclock {
    f64 start_time;
    f64 elapsed;
} hclock;

// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non started clocks.
HAPI void updateClock(hclock* clock);

// Starts the provided clock. Resets elapsed time.
HAPI void startClock(hclock* clock);

// Stops the provided clock. Does not reset elapsed time.
HAPI void stopClock(hclock* clock);

#ifdef __cplusplus
} 
#endif