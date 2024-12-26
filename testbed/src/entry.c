// entry.c

#include "game.h"
#include <entry.h>

// Define the function to create a game
b8 createGame(game* outGame) {
    // Application configuration
    outGame->config.startPosX = 100;
    outGame->config.startPosY = 100;
    outGame->config.startWidth = 720;
    outGame->config.startHeight = 480;
    outGame->config.name = "Hazker Engine Testbed";
    outGame->update = game_update;
    outGame->render = game_render;
    outGame->initialize = game_initialize;
    outGame->onResize = game_on_resize;

    // Create the game state.
    outGame->state = Hallocate(sizeof(gameState), MEMORY_TAG_GAME);
    HDEBUG("Game Instance created successfully");

    return true;
}
