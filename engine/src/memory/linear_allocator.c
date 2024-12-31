#include "memory/linear_allocator.h"
#include "memory/hmemory.h"

#include "core/logger.h"

void create_linear_allocator(u64 total_size, void* memory, linear_allocator* out_allocator) {
    if (out_allocator) {
        out_allocator->total_size = total_size;
        out_allocator->allocated = NULL;
        out_allocator->owns_memory = (memory == NULL);

        if (memory) {
            out_allocator->memory = memory;
        }
        else {
            out_allocator->memory = Hallocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}

void destroy_linear_allocator(linear_allocator* allocator) {
    if (allocator) {
        allocator->allocated = NULL;
        if (allocator->owns_memory && allocator->memory) {
            Hfree(allocator->memory, allocator->total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
        allocator->memory = NULL;
        allocator->total_size = 0;
        allocator->owns_memory = false;
    }
}

void* allocate_linear_allocator(linear_allocator* allocator, u64 size) {
    if (allocator && allocator->memory) {
        if (allocator->allocated + size > allocator->total_size) {
            u64 remaining = allocator->total_size - allocator->allocated;
            HERROR("allocate_linear_allocator - Tried to allocate %lluB, only %lluB remaining", size, remaining);
            return NULL;
        }

        void* block = ((u8*)allocator->memory) + allocator->allocated;
        allocator->allocated += size;
        return block;
    }

    HERROR("allocate_linear_allocator - Provided allocator was not initialized");
    return NULL;
}

void linear_allocator_free_all(linear_allocator* allocator) {
    if (allocator && allocator->memory) {
        allocator->allocated = NULL;
        HzeroMemory(allocator->memory, allocator->total_size);
    }
}