#include "core/hmemory.h"

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

static struct memoryStats stats;

void initializeMemory() {
    platformZeroMemory(&stats, sizeof(stats));
}

void shutdownMemory() {

}

void* Hallocate(u64 size, memoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        HWARNING("Hallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation");
    }

    stats.totalAllocated += size;
    stats.taggedAllocations[tag] += size;

    // TODO: Memory allignment
    void* block = platformAllocate(size, false);
    platformZeroMemory(block, size);

    return block;
}

void Hfree(void* block, u64 size, memoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        HWARNING("Hfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation");
    }

    stats.totalAllocated -= size;
    stats.taggedAllocations[tag] -= size;
    platformFree(block, false);
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
    u64 offset = strlen(buffer);

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; i++) {
        char unit[4] = "KiB";
        float amount = 1.0f;

        if (stats.taggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.taggedAllocations[i] / (float)gib;
        }
        else if (stats.taggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.taggedAllocations[i] / (float)mib;
        }
        else if (stats.taggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.taggedAllocations[i] / (float)kib;
        }
        else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* outString = string_duplicate(buffer);
    return outString;
}