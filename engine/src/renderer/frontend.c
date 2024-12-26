#include "renderer/frontend.h"
#include "renderer/backend.h"

#include "core/logger.h"
#include "core/hmemory.h"

// Backend render context
static rendererBackend* backend = NULL;

b8 initRenderer(const char* appName, struct platformState* plat_state) {
    backend = Hallocate(sizeof(rendererBackend), MEMORY_TAG_RENDERER);

    // TODO: Make this configurable
    rendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, plat_state, backend);
    backend->frame_number = 0;

    if(!backend->initialize(backend, appName, plat_state)) {
        HFATAL("Renderer backend failed to initialize. Shutting down");
        return false;
    }

    return true;
}

void shutdownRenderer() {
    backend->shutdown(backend);
    Hfree(backend, sizeof(rendererBackend), MEMORY_TAG_RENDERER);
}

b8 rendererBeginFrame(f32 delta_t) {
    return backend->beginFrame(backend, delta_t);
}

b8 rendererEndFrame(f32 delta_t) {
    b8 result = backend->endFrame(backend, delta_t);
    backend->frame_number++;
    return result;
}

void rendererOnResize(u16 width, u16 height) {
    if (backend) {
        backend->resized(backend, width, height);
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