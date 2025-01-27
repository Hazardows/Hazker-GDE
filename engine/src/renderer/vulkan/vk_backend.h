#include "renderer/backend.h"

b8 vk_render_backend_init(struct rendererBackend* backend, const char* app_name);
void vk_render_backend_shutdown(struct rendererBackend* backend);

void vk_render_backend_on_resized(struct rendererBackend* backend, u16 width, u16 height);

b8 vk_render_backend_begin_frame(struct rendererBackend* backend, f32 delta_t);
b8 vk_render_backend_end_frame(struct rendererBackend* backend, f32 delta_t);