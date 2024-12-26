#include "game.h"

#include "cpp_functions.hpp"
#include <stdio.h>

b8 game_initialize(game* gameInst) {
    printf("Calling C++ function from C code:\n");
    cpp_function();

    return true;
}

b8 game_update(game* gameInst, f32 deltaTime) {
    return true;
}

b8 game_render(game* gameInst, f32 deltaTime) {
    return true;
}

void game_on_resize(game* gameInst, u32 width, u32 height) {
    
}