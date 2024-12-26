#pragma once

#include "core/app.h"
#include "core/logger.h"
#include "game_types.h"
#include "core/hmemory.h"

//Externally defined function to create a game
extern b8 createGame(game* outGame);

//    * * * * * * * * * * * * * * * * * * * * * * *    //
//    *  The main entry point of the application  *    //
//    * * * * * * * * * * * * * * * * * * * * * * *    //

int main(void) {

    initializeMemory();

    //Request the game instance from the application
    game gameInstance;
    if(!createGame(&gameInstance)) {
        HFATAL("Couldn't create the game!");
        return -1;
    }

    //Ensure the game function pointers exist
    if(!gameInstance.render || !gameInstance.update || !gameInstance.initialize || !gameInstance.onResize) {
        HFATAL("The game's function pointers must be assigned!");
        return -2;
    }

    //Initialization
    if(!appCreate(&gameInstance)) {
        HINFO("Application failed to create!");
        return 1;
    }
    
    //Beguin the game loop
   
    if(!appRun()) {
        HINFO("Application did not shutdown gracefully");
        return 2;
    }

    shutdownMemory();
}