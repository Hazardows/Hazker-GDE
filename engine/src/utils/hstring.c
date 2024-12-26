#include "utils/hstring.h"
#include "core/hmemory.h"

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