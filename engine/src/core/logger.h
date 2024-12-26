#pragma once

#include "defines.h"

#define LOG_WARNING_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

//Disable debugg and trace logging for release builds
#if RELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} log_level;

b8 initLog();
void shutdownLog();

HAPI void logOutput(log_level level, const char* message, ...);

//Logs a fatal-level message
#define HFATAL(message, ...) logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef HERROR
//Logs an error-level message
#define HERROR(message, ...) logOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

//*Optional Logs*
#if LOG_WARNING_ENABLED == true
//Logs a warning-level message
#define HWARNING(message, ...) logOutput(LOG_LEVEL_WARNING, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_WARNING_ENABLED != 1
#define HWARNING(message, ...)
#endif

#if LOG_INFO_ENABLED == true
//Logs an info-level message
#define HINFO(message, ...) logOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_INFO_ENABLED != 1
#define HINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == true
//Logs a debug-level message
#define HDEBUG(message, ...) logOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_DEBUG_ENABLED != 1
#define HDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == true
//Logs a trace-level message
#define HTRACE(message, ...) logOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_TRACE_ENABLED != 1
#define HTRACE(message, ...)
#endif