#pragma once

#ifdef __cplusplus 
extern "C" { 
#endif

#include "defines.h"

// Disable assertions by commenting out the below line
#define HASSERTIONS_ENABLED

#ifdef HASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

HAPI void reportAssertionFailure(const char* expression, const char* message, const char* file, i32 line);

#define HASSERT(expr)                                           \
    if (!(expr)) {                                              \
        reportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
        debugBreak();                                           \
    }

#define HASSERT_MSG(expr, message)                                  \
    if (!(expr)) {                                                  \
        reportAssertionFailure(#expr, message, __FILE__, __LINE__); \
        debugBreak();                                               \
    }

#ifdef DEBUG
#define HASSERT_DEBUG(expr)                                    \
    if (!(expr)) {                                             \
        reportAssertionFailure(#expr, "", __FILE__, __LINE__); \
        debugBreak();                                          \
    }
#else
#define HASSERT_DEBUG(expr) // Does nothing
#endif

#else
#define HASSERT(expr)                 // Does nothing
#define HASSERT_MSG(expr, message)    // Does nothing
#define HASSERT_DEBUG(expr)           // Does nothing
#endif

#ifdef __cplusplus
} 
#endif