#pragma once

#include "defines.h"

#ifdef __cplusplus 
extern "C" { 
#endif

// Returns the length of the given string
HAPI u64 string_length(const char* str);

HAPI char* string_duplicate(const char* str);

// Case-sensitive string comparison. true if the same, otherwise false.
HAPI b8 strings_equal(const char* str1, const char* str2);

// Perform string formatting to dest given format string and parameters.
HAPI i32 string_format(char* dest, const char* format, ...);

/**
 * Performs variadic string formatting to dest given format string and va_list.
 * @param dest The destination for the formatted string.
 * @param format The string to be formatted.
 * @param va_list The variadic argument list.
 * @returns The size of the data written.
 */
HAPI i32 string_format_v(char* dest, const char* format, void* va_list);

#ifdef __cplusplus
} 
#endif