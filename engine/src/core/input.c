#include "core/input.h"
#include "core/events.h"
#include "memory/hmemory.h"
#include "core/logger.h"

typedef struct keyboardState {
    b8 keys[256];
} keyboardState;

typedef struct mouseState {
    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
} mouseState;

typedef struct inputState {
    keyboardState kcur;
    keyboardState kprev;
    mouseState mcur;
    mouseState mprev;
} inputState;

static b8 initialized = false;
static inputState state = {};

void inputInit() {
    HzeroMemory(&state, sizeof(inputState));
    initialized = true;
    HINFO("Input subsystem initialized")
}

void inputShutdown() {
    // TODO: Add shutdown routines when needed
    initialized = false;
}

void inputUpdate(f64 deltaTime) {
    if(!initialized) {
        return;
    }

    // Copy current state to previous state
    HcopyMemory(&state.kprev, &state.kcur, sizeof(keyboardState));
    HcopyMemory(&state.mprev, &state.mcur, sizeof(mouseState));
}

void input_process_key(keys key, b8 pressed) {
    if (key == KEY_LALT) {
        HINFO("Left alt pressed/released");
    }
    else if (key == KEY_RALT) {
        HINFO("Right alt pressed/released");
    }
    else if (key == KEY_LCONTROL) {
        HINFO("Left ctrl pressed/released");
    }
    else if (key == KEY_RCONTROL) {
        HINFO("Right ctrl pressed/released");
    }
    else if (key == KEY_LSHIFT) {
        HINFO("Left shift pressed/released");
    }
    else if (key == KEY_RSHIFT) {
        HINFO("Right shift pressed/released");
    }

    // Only handle this if the state was actually changed
    if (state.kcur.keys[key] != pressed) {
        // Update internal state
        state.kcur.keys[key] = pressed;

        

        // Fire an event for inmediate processing
        eventContext context;
        context.data.u16[0] = key;
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void input_process_button(buttons button, b8 pressed) {
    // If the state changed, fire an event
    if (state.mcur.buttons[button] != pressed) {
        state.mcur.buttons[button] = pressed;

        // Fire the event
        eventContext context;
        context.data.u16[0] = button;
        eventFire(pressed ? EVENT_CODE_MOUSE_BUTTON_PRESSED : EVENT_CODE_MOUSE_BUTTON_RELEASED, 0, context);
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    // Only proccess if actually different
    if (state.mcur.x != x || state.mcur.y != y) {
        // NOTE: Enable this if debugging
        // HDEBUG("Mouse pos: %i, %i.", x, y);

        // Update internal state
        state.mcur.x = x;
        state.mcur.y = y;

        // Fire event
        eventContext context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        eventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void input_process_mouse_wheel(i8 z_delta) {
    // NOTE: No internal state to update

    // Fire event
    eventContext context;
    context.data.u8[0] = z_delta;
    eventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 input_key_down(keys key) {
    if(!initialized) {
        return false;
    }
    return state.kcur.keys[key];
}

b8 input_key_up(keys key) {
    if(!initialized) {
        return false;
    }
    return !state.kcur.keys[key];
}

b8 input_was_key_down(keys key) {
    if(!initialized) {
        return false;
    }
    return state.kprev.keys[key];
}

b8 input_was_key_up(keys key) {
    if(!initialized) {
        return false;
    }
    return !state.kprev.keys[key];
}

b8 input_is_button_down(buttons button) {
    if(!initialized) {
        return false;
    }
    return state.mcur.buttons[button];
}

b8 input_is_button_up(buttons button) {
    if(!initialized) {
        return false;
    }
    return !state.mcur.buttons[button];
}

b8 input_was_button_down(buttons button) {
    if(!initialized) {
        return false;
    }
    return state.mprev.buttons[button];
}

b8 input_was_button_up(buttons button) {
    if(!initialized) {
        return false;
    }
    return !state.mprev.buttons[button];
}

void input_get_mouse_possition(i32* x, i32* y) {
    if(!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mcur.x;
    *y = state.mcur.y;
}

void input_get_previus_mouse_possition(i32* x, i32* y) {
    if(!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mprev.x;
    *y = state.mprev.y;
}