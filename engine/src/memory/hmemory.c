#include "memory/hmemory.h"

#include "core/logger.h"
#include "platform/platform.h"
#include "utils/hstring.h"

#include <stdio.h>

struct memoryStats {
    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "LINEAR ALLC",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      ",
    "EVENT      "
};

typedef struct memory_system_state {
    struct memoryStats stats;
    u64 alloc_count;
} memory_system_state;

static memory_system_state* state_ptr;

void initializeMemory(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(memory_system_state);
    if (state == NULL) {
        return;
    } 

    state_ptr = state;
    state_ptr->alloc_count = 0;

    platformZeroMemory(&state_ptr->stats, sizeof(state_ptr->stats));
}

void shutdownMemory(void *state) {
    state_ptr = NULL;
}

void* Hallocate(u64 size, memoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        HWARNING("Hallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation");
    }

    if (state_ptr) {
        state_ptr->stats.totalAllocated += size;
        state_ptr->stats.taggedAllocations[tag] += size;
        state_ptr->alloc_count++;
    }

    // TODO: Memory allignment
    void* block = platformAllocate(size, false);
    platformZeroMemory(block, size);

    return block;
}

void Hfree(void* block, u64 size, memoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        HWARNING("Hfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation");
    }

    if (state_ptr) {
        state_ptr->stats.totalAllocated -= size;
        state_ptr->stats.taggedAllocations[tag] -= size;
        platformFree(block, false);
    }
}

void* HzeroMemory(void* block, u64 size) {
    return platformZeroMemory(block, size);
}

void* HcopyMemory(void* dest, const void* source, u64 size) {
    return platformCopyMemory(dest, source, size);
}

void* HsetMemory(void* dest, i32 value, u64 size) {
    return platformSetMemory(dest, value, size);
}

char* GetMemoryUsage_str() {
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System Memory Usage (tagged):\n";
    u64 offset = string_length(buffer);

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; i++) {
        char unit[4] = "KiB";
        float amount = 1.0f;

        if (state_ptr->stats.taggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = state_ptr->stats.taggedAllocations[i] / (float)gib;
        }
        else if (state_ptr->stats.taggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = state_ptr->stats.taggedAllocations[i] / (float)mib;
        }
        else if (state_ptr->stats.taggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = state_ptr->stats.taggedAllocations[i] / (float)kib;
        }
        else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)state_ptr->stats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* outString = string_duplicate(buffer);
    return outString;
}

u64 GetMemoryAllocCount() {
    if (state_ptr) {
        return state_ptr->alloc_count;
    }
    return 0;
}