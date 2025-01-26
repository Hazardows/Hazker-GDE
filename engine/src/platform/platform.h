#pragma once

#ifdef __cplusplus 
extern "C" { 
#endif

#include "defines.h"

typedef struct platformState {
    void* internalState;
} platformState;

b8 platformStartup(
    platformState* pState,
    const char* appName, 
    i32 x, 
    i32 y, 
    i32 width,
    i32 height
);

void platformShutdown(platformState* pState);
b8 platformPumpMessages(platformState* pState);

//Dealing with memory
void* platformAllocate(u64 size, b8 aligned);
void platformFree(void* block, b8 aligned);
void* platformZeroMemory(void* block, u64 size);
void* platformCopyMemory(void* dest, const void* source, u64 size);
void* platformSetMemory(void* dest, i32 value, u64 size);

//Console Messages
void platformConsoleWrite(const char* message, u8 colour);
void platformConsoleWriteError(const char* message, u8 colour);

//Obtaining time
f64 platformGetAbsoluteTime();

//Sleep on the thread for the provided ms, blocks the main thread.
//Use only for giving time back to the OS for unused update power.
//Therefore is not exported
void platformSleep(u64 ms);

#ifdef __cplusplus
} 
#endif