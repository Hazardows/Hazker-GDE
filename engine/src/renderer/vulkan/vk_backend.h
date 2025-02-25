#include "renderer/backend.h"

b8 vk_renderer_backend_init(struct rendererBackend* backend, const char* app_name);
void vk_renderer_backend_shutdown(struct rendererBackend* backend);

void vk_renderer_backend_on_resized(struct rendererBackend* backend, u16 width, u16 height);

b8 vk_renderer_backend_begin_frame(struct rendererBackend* backend, f32 delta_t);
void vk_renderer_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
b8 vk_renderer_backend_end_frame(struct rendererBackend* backend, f32 delta_t);

void vk_backend_update_object(mat4 model);