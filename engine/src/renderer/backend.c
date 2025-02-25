#include "renderer/backend.h"

#include "renderer/vulkan/vk_backend.h"

b8 rendererBackendCreate(rendererBackendType type, rendererBackend* out_render_backend) {
    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        out_render_backend->initialize = vk_renderer_backend_init;
        out_render_backend->shutdown = vk_renderer_backend_shutdown;
        out_render_backend->resized = vk_renderer_backend_on_resized;
        out_render_backend->beginFrame = vk_renderer_backend_begin_frame;
        out_render_backend->updateGlobalState = vk_renderer_update_global_state;
        out_render_backend->endFrame = vk_renderer_backend_end_frame;
        out_render_backend->updateObject = vk_backend_update_object;

        return true;
    }

    return false;
}

void rendererBackendDestroy(rendererBackend* backend) {
    backend->initialize = NULL;
    backend->shutdown = NULL;
    backend->beginFrame = NULL;
    backend->updateGlobalState = NULL;
    backend->endFrame = NULL;
    backend->resized = NULL;
    backend->updateObject = NULL;
}