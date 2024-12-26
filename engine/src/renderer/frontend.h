#pragma once

#include "renderer/types.inl"

struct staticMeshData;
struct platformState;

b8 initRenderer(const char* appName, struct platformState* plat_state);
void shutdownRenderer();

void rendererOnResize(u16 width, u16 height);

b8 rendererDrawFrame(renderPacket* packet);