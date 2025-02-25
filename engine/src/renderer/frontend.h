#pragma once

#include "renderer/types.inl"

b8 initRenderer(u64* memory_requirement, void* state, const char* appName);
void shutdownRenderer(void* state);

void rendererOnResize(u16 width, u16 height);

b8 rendererDrawFrame(renderPacket* packet);

// HACK: This should not be exposed outside the engine
HAPI void rendererSetView(mat4 view);