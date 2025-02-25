#include "utils/hstring.h"
#include "memory/hmemory.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

char* string_duplicate(const char* str) {
    u64 len = string_length(str);
    char* copy = Hallocate(len + 1, MEMORY_TAG_STRING);
    HcopyMemory(copy, str, len + 1);
    return copy;
}

u64 string_length(const char* str) {
    return strlen(str);
}

b8 strings_equal(const char* str1, const char* str2) {
    return !strcmp(str1, str2);
}

i32 string_format(char* dest, const char* format, ...) {
    if (dest) {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = string_format_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 string_format_v(char* dest, const char* format, void* va_listp) {
    if (dest) {
        // Big, but can fit on the stack.
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, va_listp);
        buffer[written] = 0;
        HcopyMemory(dest, buffer, written + 1);

        return written;
    }
    return -1;
}