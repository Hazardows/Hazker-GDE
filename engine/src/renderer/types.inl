#pragma once

#include "defines.h"
#include "math/math_types.inl"

typedef enum rendererBackendType {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} rendererBackendType;

typedef struct globalUniformObject {
    // this must be 256 bytes aligned
    mat4 projection;    // (64 bytes) projection matrix
    mat4 view;          // (64 bytes) view matrix
    mat4 m_reserved0;   // (64 bytes) reserved for future use
    mat4 m_reserved1;   // (64 bytes) reserved for future use
} globalUniformObject;

typedef struct rendererBackend {
    u64 frame_number;

    b8 (*initialize)(struct rendererBackend* backend, const char* app_name);
    
    void (*shutdown)(struct rendererBackend* backend);

    void (*resized)(struct rendererBackend* backend, u16 width, u16 height);

    b8 (*beginFrame)(struct rendererBackend* backend, f32 delta_t);
    void (*updateGlobalState)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
    b8 (*endFrame)(struct rendererBackend* backend, f32 delta_t);

    void (*updateObject)(mat4 model);
} rendererBackend;

typedef struct renderPacket {
    f32 delta_time;
} renderPacket;
