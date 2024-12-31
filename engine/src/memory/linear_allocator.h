#pragma once

#ifdef __cplusplus 
extern "C" { 
#endif

#include "defines.h"

typedef struct linear_allocator {
    u64 total_size;
    u64 allocated;
    void* memory;
    b8 owns_memory;
} linear_allocator;

HAPI void create_linear_allocator(u64 total_size, void* memory, linear_allocator* out_allocator);
HAPI void destroy_linear_allocator(linear_allocator* allocator);

HAPI void* allocate_linear_allocator(linear_allocator* allocator, u64 size);
HAPI void linear_allocator_free_all(linear_allocator* allocator);

#ifdef __cplusplus
} 
#endif