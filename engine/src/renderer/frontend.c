#include "renderer/frontend.h"
#include "renderer/backend.h"

#include "core/logger.h"
#include "memory/hmemory.h"

typedef struct rendererSystemState {
    // Backend render context
    rendererBackend backend;
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
        state_ptr->backend.resized(&state_ptr->backend, width, height);
    }
    else {
        HWARNING("Renderer backend does not exist to accept resize: %i %i", width, height);
    }
}

b8 rendererDrawFrame(renderPacket* packet) {
    // If the begin returned successfully, mid frame operations may continue.
    if (rendererBeginFrame(packet->delta_time)) {
        
        // End the frame. If this fails, it is likely unrecoverable.
        b8 result = rendererEndFrame(packet->delta_time);
        
        if (!result) {
            HERROR("rendererEndFrame falied. Shutting down application...");       
            return false;
        }
    }

    return true;
}