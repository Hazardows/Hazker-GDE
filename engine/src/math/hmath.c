#include "math/hmath.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

// ****************************************************************** //
// *  Note that this are here in order to prevent having to import  * //
// *  the entire <math.h> everywhere.                               * //
// ****************************************************************** //

static b8 rand_seeded = false;

f32 hsin(f32 x) {
    return sinf(x);
}

f32 hcos(f32 x) {
    return cosf(x);
}

f32 htan(f32 x) {
    return tanf(x);
}

f32 hacos(f32 x) {
    return acosf(x);
}

f32 hsqrt(f32 x) {
    return sqrtf(x);
}

f32 habs(f32 x) {
    return fabsf(x);
}

i32 hrandom() {
    if (!rand_seeded) {
        srand((u32)platformGetAbsoluteTime());
        rand_seeded = true;
    }
    return rand();
}

i32 hrandomInRange(i32 min, i32 max) {
    if (!rand_seeded) {
        srand((u32)platformGetAbsoluteTime());
        rand_seeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 fhrandom() {
    return (float)hrandom() / (f32)RAND_MAX;
}

f32 fhrandomInRange(f32 min, f32 max) {
    return min + ((float)hrandom() / ((f32)RAND_MAX / (max - min)));
}