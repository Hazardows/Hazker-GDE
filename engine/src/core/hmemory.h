#pragma once

#include "defines.h"

typedef enum memoryTag {
    // For temporary use. Should be assigned to one of the bellow or have a new tag created
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    // TODO: MEMORY_TAG_BLOWJOB, (Just joking)
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_EVENT,

    MEMORY_TAG_MAX_TAGS
} memoryTag;

HAPI void initializeMemory();
HAPI void shutdownMemory();

HAPI void* Hallocate(u64 size, memoryTag tag);

HAPI void Hfree(void* block, u64 size, memoryTag tag);

HAPI void* HzeroMemory(void* block, u64 size);

HAPI void* HcopyMemory(void* dest, const void* source, u64 size);

HAPI void* HsetMemory(void* dest, i32 value, u64 size);

HAPI char* GetMemoryUsage_str();