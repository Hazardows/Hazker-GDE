#pragma once

#include "defines.h"

// Temporal definition to allow compiling
#define alignas(i)

typedef union vec2_u {
    // An array of x, y.
    f32 elements[2];
    struct {
        union {
            // First element.
            f32 x, r, s, u;
        };
        union {
            // Second element.
            f32 y, g, t, v;
        };
    };
} vec2;

typedef union vec3_u {
    // An array of x, y, z.
    f32 elements[3];
    struct {
        union {
            // First element.
            f32 x, r, s, u;
        };
        union {
            // Second element.
            f32 y, g, t, v;
        };
        union {
            // Third element.
            f32 z, b, p, w;
        };
    };
} vec3;

typedef union vec4_u {
#if defined(HUSE_SIMD) 
    // Used for SIMD operations.
    alignas(16) __m128 data;
#endif
    // An array of x, y, z, w.
    alignas(16) f32 elements[4];
    union {
        union {
            // First element.
            f32 x, r, s;
        };
        union {
            // Second element.
            f32 y, g, t;
        };
        union {
            // Third element.
            f32 z, b, p;
        };
        union {
            // Fourth element.
            f32 w, a, q;
        };
    };
} vec4;

typedef vec4 quat;