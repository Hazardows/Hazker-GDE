#pragma once

#include <game_types.h>
#include <math/hmath.h>

typedef struct gameState {
    f32 deltaTime;
    
    mat4 view;

    vec3 camera_position;
    vec3 camera_euler;

    b8 camera_view_dirty;
} gameState;

b8 game_initialize(game* gameInstance);

b8 game_update(game* gameInstance, f32 deltaTime);

b8 game_render(game* gameInstance, f32 deltaTime);

void game_on_resize(game* gameInstance, u32 width, u32 height);