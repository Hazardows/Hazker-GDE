#pragma once

#include "defines.h"

typedef enum rendererBackendType {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} rendererBackendType;

typedef struct rendererBackend {
    struct platformState* plat_state;
    u64 frame_number;

    b8 (*initialize)(struct rendererBackend* backend, const char* app_name);
    
    void (*shutdown)(struct rendererBackend* backend);

    void (*resized)(struct rendererBackend* backend, u16 width, u16 height);

    b8 (*beginFrame)(struct rendererBackend* backend, f32 delta_t);
    b8 (*endFrame)(struct rendererBackend* backend, f32 delta_t);
} rendererBackend;

typedef struct renderPacket {
    f32 delta_time;
} renderPacket;
