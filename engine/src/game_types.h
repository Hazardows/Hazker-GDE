#pragma once

//    * * * * * * * * * * * * * * * * * * * * * * * * *    //
//    *   Represents the basic game state in a game.  *    //
//    *    Called for creation by the application.    *    //
//    * * * * * * * * * * * * * * * * * * * * * * * * *    //

#include "core/app.h"

typedef struct game {
    // The application configuration
    appConfig config;

    //Function pointer to game's initialize function
    b8 (*initialize)(struct game* gameInstance);

    //Function pointer to game's update function
    b8 (*update)(struct game* gameInstance, f32 deltaTime);

    //Function pointer to game's render function
    b8 (*render)(struct game* gameInstance, f32 deltaTime);

    //Function pointer to handle resizes, if applicable
    void (*onResize)(struct game* gameInstance, u32 width, u32 height);

    //Game specific game state, Created and managed by the game
    void* state;

    // Application State.
    void* appState;
} game;