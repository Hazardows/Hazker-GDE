#include "core/logger.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef struct logger_system_state {
    b8 initialized;
} logger_system_state;

static logger_system_state* state_ptr;

b8 initLog(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;
    state_ptr->initialized = true;

    // TODO: create log file.
    return true;
}

void shutdown_logging(void* state) {
    // TODO: cleanup logging/write queued entries.
    state_ptr = 0;
}

void logOutput(log_level level, const char* message, ...) {
    const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARNING]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    b8 isError = (level < LOG_LEVEL_WARNING);

    //Technically imposes a 32k character limit on a single log entry, but...
    //It's not recomended to do this
    const i32 msgLength = 32000;
    char outMessage[msgLength];
    memset(outMessage, 0, sizeof(outMessage));

    //Format original message
    __builtin_va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, 32000, message, argPtr);
    va_end(argPtr);

    char outMessage2[32000];
    sprintf(outMessage2, "%s%s\n", levelStrings[level], outMessage);

    // Platform-specific output
    if(isError) platformConsoleWriteError(outMessage2, level);
    else platformConsoleWrite(outMessage2, level);
}