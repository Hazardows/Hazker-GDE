#include "core/asserts.h"
#include "core/logger.h"

/**
 * Reports an assertion failure with details about the expression, message, file, and line.
 * @param expression The expression that failed the assertion.
 * @param message An optional message providing additional details.
 * @param file The file in which the assertion failed.
 * @param line The line number at which the assertion failed.
 */
void reportAssertionFailure(const char* expression, const char* message, const char* file, i32 line) {
    logOutput(LOG_LEVEL_FATAL, "AssertionFailure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}
