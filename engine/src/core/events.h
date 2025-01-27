#pragma once

#ifdef __cplusplus 
extern "C" { 
#endif

#include "defines.h"

typedef struct eventContext { // *Without pretext*
    // 128 bytes
    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;
} eventContext;

void eventInit(u64* memory_requirement, void* state);
void eventShutdown(void* state);

//Should return true if handled
typedef b8 (*PFNC_onEvent)(u16 code, void* sender, void* listenerInstance, eventContext data);

/**
 * Register to listen for when events are sent with the provided code. Events with duplicate
 * listener/callback combos will not be registered again will cause this to return false.
 * @param code The event code to listen for.
 * @param listener A pointer to a listener instance (Can be 0/NULL).
 * @param onEvent The callback function pointer to be invoked when event code is fired.
 * @returns true if the event was successfully registered; otherwise false.
 */
HAPI b8 eventRegister(u16 code, void* listener, PFNC_onEvent onEvent);

/**
 * Unregister from listening for when events are sent with the provided code.
 * If no matching registration is found, this function return false.
 * @param code The event code to stop listening for.
 * @param listener A pointer to a listener instance (Can be 0/NULL).
 * @param onEvent The callback function pointer to be unregistered.
 * @returns true if the event was successfully unregistered; otherwise false.
 */
HAPI b8 eventUnregister(u16 code, void* listener, PFNC_onEvent onEvent);

/**
 * Fires an event to the listeners of the given code.
 * If an event returns true, the event is considered handled and is not apssed on to any more listeners.
 * @param code The event code to fire.
 * @param listener A pointer to the sender (Can be 0/NULL).
 * @param data The event data
 * @returns true if handled; otherwise false.
 */
HAPI b8 eventFire(u16 code, void* sender, eventContext context);

// System internal event codes. Application should use codes beyond 255.
typedef enum systemEventCode {
    // Shuts the application down on the next frame.
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    // Keyboard key pressed
    EVENT_CODE_KEY_PRESSED = 0x02, 
    /* Context usage:
     * u16 keyCode = data.data.u16[0];
    */
    // Keyboard key released
    EVENT_CODE_KEY_RELEASED = 0x03, 
    /* Context usage:
     * u16 keyCode = data.data.u16[0];
    */

    // Mouse button pressed
    EVENT_CODE_MOUSE_BUTTON_PRESSED = 0x04,
    /* Context usage:
     * u16 keyCode = data.data.u16[0];
    */
    // Mouse button released
    EVENT_CODE_MOUSE_BUTTON_RELEASED = 0x05,
    /* Context usage:
     * u16 keyCode = data.data.u16[0];
    */
    // Mouse moved
    EVENT_CODE_MOUSE_MOVED = 0x06,
    /* Context usage:
     * u16 x = data.data.u16[0];
     * u16 y = data.data.u16[1];
    */
    // Mouse wheel moved
    EVENT_CODE_MOUSE_WHEEL = 0x07,
    /* Context usage:
     * u8 z_delta = data.data.u8[0];
    */

    // Resized/resolution changed from the OS
    EVENT_CODE_RESIZED = 0x08,
    /* Context usage:
     * u16 widht = data.data.u16[0];
     * u16 height = data.data.u16[1];
    */

    MAX_EVENT_CODE = 0xFF
} systemEventCode;

#ifdef __cplusplus
} 
#endif