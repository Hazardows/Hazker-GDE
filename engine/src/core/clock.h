#pragma once

#ifdef __cplusplus 
extern "C" { 
#endif

#include "defines.h"

typedef struct clock {
    f64 start_time;
    f64 elapsed;
} clock;

// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non started clocks.
HAPI void updateClock(clock* clock);

// Starts the provided clock. Resets elapsed time.
HAPI void startClock(clock* clock);

// Stops the provided clock. Does not reset elapsed time.
HAPI void stopClock(clock* clock);

#ifdef __cplusplus
} 
#endif