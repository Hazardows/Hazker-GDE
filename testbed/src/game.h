#pragma once

#include <game_types.h>

typedef struct gameState {
    f32 deltaTime;
} gameState;

b8 game_initialize(game* gameInstance);

b8 game_update(game* gameInstance, f32 deltaTime);

b8 game_render(game* gameInstance, f32 deltaTime);

void game_on_resize(game* gameInstance, u32 width, u32 height);