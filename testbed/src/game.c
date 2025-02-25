#include "game.h"

#include "cpp_functions.hpp"
#include <stdio.h>

#include <core/input.h>
#include <core/logger.h>
#include <memory/hmemory.h>

// HACK: This should not be available outside the engine
#include <renderer/frontend.h>

// HCLAMP
#include <utils/utils.h>

// Global temporary variable
gameState* state;

void recalculate_view_matrix() {
    if (state->camera_view_dirty) {
        mat4 rotation = mat4_euler_xyz(
            state->camera_euler.x,
            state->camera_euler.y,
            state->camera_euler.z
        );
        mat4 translation = mat4_translation(state->camera_position);

        state->view = mat4_mul(rotation, translation);
        state->view = mat4_inverse(state->view);

       state->camera_view_dirty = false;
    }
}

void camera_yaw(f32 amount) {
    state->camera_euler.y += amount;
    state->camera_view_dirty = true;
}
void camera_pitch(f32 amount) {
    state->camera_euler.x += amount;

    // Clamp to avoid Gimball lock
    f32 limit = deg_to_rad(89.0f);
    state->camera_euler.x = HCLAMP(state->camera_euler.x, -limit, limit);

    state->camera_view_dirty = true;
}

b8 game_initialize(game* gameInst) {
    printf("Calling C++ function from C code:\n");
    cpp_function();

    state = (gameState*)(gameInst->state);

    state->camera_position = (vec3){0, 0, 20.0f};
    state->camera_euler = vec3_zero();

    state->view = mat4_translation(state->camera_position);
    state->view = mat4_inverse(state->view);

    state->camera_view_dirty = true;

    return true;
}

b8 game_update(game* gameInst, f32 deltaTime) {
    static u64 allocCount = 0;
    u64 prevAllocCount = allocCount;
    allocCount = GetMemoryAllocCount();
    if (keyJustPressed(KEY_M)) {
        HDEBUG("Allocations: %llu (%llu this frame)", allocCount, allocCount - prevAllocCount);
    }

    // HACK: temp hack to move camera around
    if (keyPressed(KEY_LEFT))  camera_yaw(1.0f * deltaTime);
    if (keyPressed(KEY_RIGHT)) camera_yaw(-1.0f * deltaTime);
    if (keyPressed(KEY_UP))    camera_pitch(1.0f * deltaTime);
    if (keyPressed(KEY_DOWN))  camera_pitch(-1.0f * deltaTime);

    f32 move_speed = 25.0f;
    vec3 velocity = vec3_zero();

    if (keyPressed(KEY_W)) {
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }
    if (keyPressed(KEY_S)) {
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }
    if (keyPressed(KEY_A)) {
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }
    if (keyPressed(KEY_D)) {
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }

    if (keyPressed(KEY_SPACE)) {
        velocity.y += 1.0f;
    }
    if (keyPressed(KEY_LCONTROL)) {
        velocity.y -= 1.0f;
    }

    if (!vec3_compare(vec3_zero(), velocity, 0.0002f)) {
        // Normalize velocity before applying speed
        vec3_normalize(&velocity);
        state->camera_position.x += velocity.x * move_speed * deltaTime;
        state->camera_position.y += velocity.y * move_speed * deltaTime;
        state->camera_position.z += velocity.z * move_speed * deltaTime;
        state->camera_view_dirty = true;
    }

    recalculate_view_matrix();

    // Mouse buttton messages
    cpp_button();

    // HACK: This should not be available outside the engine
    rendererSetView(state->view);

    return true;
}

b8 game_render(game* gameInst, f32 deltaTime) {
    return true;
}

void game_on_resize(game* gameInst, u32 width, u32 height) {
    
}