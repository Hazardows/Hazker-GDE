#include "core/logger.h"
#include "core/asserts.h"
#include "memory/hmemory.h"

#include "platform/platform.h"
#include "platform/filesystem.h"

#include "utils/hstring.h"

// TODO: temporary
#include <stdarg.h>

typedef struct logger_system_state {
    fileHandle log_file_handle;
} logger_system_state;

static logger_system_state* state_ptr;

void append_to_log_file(const char* message) {
    if (state_ptr && state_ptr->log_file_handle.isValid) {
        // Since the message already contains a '\n', just write bytes directly.
        u64 length = string_length(message);
        u64 writen = 0;
        if (!filesystem_write(&state_ptr->log_file_handle, length, message, &writen)) {
            platformConsoleWriteError("ERROR writing to consoole.log for writing.", LOG_LEVEL_ERROR);
        }
    }
}

b8 initLog(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;

    // Create or wipe existing log file, then open it.
    if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
        platformConsoleWriteError("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    return true;
}

void shutdown_logging(void* state) {
    // TODO: cleanup logging/write queued entries.
    state_ptr = 0;
}

void logOutput(log_level level, const char* message, ...) {
    // TODO: This string operations are all pretty slow.
    // This will be moved in the future to another thread along with the
    // file writes to avoid slowing things down while the engine is running.
    const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARNING]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    b8 isError = (level < LOG_LEVEL_WARNING);

    //Technically imposes a 32k character limit on a single log entry, but...
    //It's not recomended to do this
    #define msgLength 32000
    char outMessage[msgLength];
    HzeroMemory(outMessage, sizeof(outMessage));

    //Format original message
    __builtin_va_list argPtr;
    va_start(argPtr, message);
    string_format_v(outMessage, message, argPtr);
    va_end(argPtr);

    char outMessage2[32000];
    string_format(outMessage2, "%s%s\n", levelStrings[level], outMessage);

    // Platform-specific output
    if(isError) platformConsoleWriteError(outMessage2, level);
    else platformConsoleWrite(outMessage2, level);

    // Queue a copy to be writen to the log file
    append_to_log_file(outMessage2);
}