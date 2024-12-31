#pragma once

#ifdef __cplusplus 
extern "C" { 
#endif

#include "defines.h"

struct game;

typedef struct appConfig {
    // Window starting position x axis, if applicable
    i16 startPosX;
    // Window starting position y axis, if applicable
    i16 startPosY;

    // Window starting width, if applicable
    i16 startWidth;
    // Window starting height, if applicable
    i16 startHeight;

    // The application name used in windowing, if applicable
    char* name;
} appConfig;

HAPI b8 appCreate(struct game* gameInstance);
HAPI b8 appRun();

void appGetFramebufferSize(u32* width, u32* height);

#ifdef __cplusplus
} 
#endif