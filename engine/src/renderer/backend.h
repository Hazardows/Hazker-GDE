#pragma once

#include "renderer/types.inl"

struct platformState;

b8 rendererBackendCreate(rendererBackendType type, struct platformState* plat_state, rendererBackend* out_render_backend);
void rendererBackendDestroy(rendererBackend* backend);