#include "renderer/frontend.h"
#include "renderer/backend.h"

#include "core/logger.h"
#include "memory/hmemory.h"
#include "math/hmath.h"

typedef struct rendererSystemState {
    // Backend render context
    rendererBackend backend;
    
    mat4 projection;
    mat4 view;

    f32 near_clip;
    f32 far_clip;
} rendererSystemState;

rendererSystemState* state_ptr;

b8 initRenderer(u64* memory_requirement, void* state, const char* appName) {
    *memory_requirement = sizeof(rendererSystemState);
    if (state == NULL) {
        return true;
    }
    state_ptr = state;
    
    // TODO: Make this configurable
    rendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, &state_ptr->backend);
    state_ptr->backend.frame_number = 0;

    if(!state_ptr->backend.initialize(&state_ptr->backend, appName)) {
        HFATAL("Renderer backend failed to initialize. Shutting down");
        return false;
    }

    // World projection/view
    state_ptr->near_clip = 0.1f;
    state_ptr->far_clip = 1000.0f;
    state_ptr->projection = mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, state_ptr->near_clip, state_ptr->far_clip);
    // TODO: configurable camera starting position.
    state_ptr->view = mat4_translation((vec3){0, 0, -30.0f});
    state_ptr->view = mat4_inverse(state_ptr->view);

    return true;
}

void shutdownRenderer(void* state) {
    if (state_ptr) {
        state_ptr->backend.shutdown(&state_ptr->backend);
    }
    state_ptr = NULL;
}

b8 rendererBeginFrame(f32 delta_t) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->backend.beginFrame(&state_ptr->backend, delta_t);
}

b8 rendererEndFrame(f32 delta_t) {
    if (!state_ptr) {
        return false;
    }
    b8 result = state_ptr->backend.endFrame(&state_ptr->backend, delta_t);
    state_ptr->backend.frame_number++;
    return result;
}

void rendererOnResize(u16 width, u16 height) {
    if (state_ptr) {
        state_ptr->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)height, state_ptr->near_clip, state_ptr->far_clip);
        state_ptr->backend.resized(&state_ptr->backend, width, height);
    }
    else {
        HWARNING("Renderer backend does not exist to accept resize: %i %i", width, height);
    }
}

b8 rendererDrawFrame(renderPacket* packet) {
    // If the begin returned successfully, mid frame operations may continue.
    if (rendererBeginFrame(packet->delta_time)) {
        state_ptr->backend.updateGlobalState(state_ptr->projection, state_ptr->view, vec3_zero(), vec4_one(), 0);

        mat4 model = mat4_translation((vec3){0, 0, 0});
        /*static f32 angle = 0.01f;
        angle += 0.001f;
        quat rotation = quat_from_axis_angle(vec3_forward(), angle, false);
        mat4 model = quat_to_rotaion_matrix(rotation, vec3_zero());*/
        state_ptr->backend.updateObject(model);

        // End the frame. If this fails, it is likely unrecoverable.
        b8 result = rendererEndFrame(packet->delta_time);
        
        if (!result) {
            HERROR("rendererEndFrame falied. Shutting down application...");       
            return false;
        }
    }

    return true;
}

void rendererSetView(mat4 view) {
    state_ptr->view = view;
}