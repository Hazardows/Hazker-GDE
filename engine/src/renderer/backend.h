#pragma once

#include "renderer/types.inl"

b8 rendererBackendCreate(rendererBackendType type, rendererBackend* out_render_backend);
void rendererBackendDestroy(rendererBackend* backend);