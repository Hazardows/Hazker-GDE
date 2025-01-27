#include "renderer/backend.h"

#include "renderer/vulkan/vk_backend.h"

b8 rendererBackendCreate(rendererBackendType type, rendererBackend* out_render_backend) {
    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        out_render_backend->initialize = vk_render_backend_init;
        out_render_backend->shutdown = vk_render_backend_shutdown;
        out_render_backend->resized = vk_render_backend_on_resized;
        out_render_backend->beginFrame = vk_render_backend_begin_frame;
        out_render_backend->endFrame = vk_render_backend_end_frame;

        return true;
    }

    return false;
}

void rendererBackendDestroy(rendererBackend* backend) {
    backend->initialize = NULL;
    backend->shutdown = NULL;
    backend->beginFrame = NULL;
    backend->endFrame = NULL;
    backend->resized = NULL;
}