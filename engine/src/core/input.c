#include "core/input.h"
#include "core/events.h"
#include "memory/hmemory.h"
#include "core/logger.h"

typedef struct keyboardState {
    b8 keys[KEYS_MAX_KEYS];
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

// Internal input state pointer
static inputState* state_ptr;

void inputInit(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(inputState);
    if (state == NULL) {
        return;
    }
    HzeroMemory(state, sizeof(inputState));
    state_ptr = state;

    HINFO("Input subsystem initialized")
}

void inputShutdown(void* state) {
    // TODO: Add shutdown routines when needed
    state_ptr = NULL;
}

void inputUpdate(f64 deltaTime) {
    if(!state_ptr) {
        return;
    }

    // Copy current state to previous state
    HcopyMemory(&state_ptr->kprev, &state_ptr->kcur, sizeof(keyboardState));
    HcopyMemory(&state_ptr->mprev, &state_ptr->mcur, sizeof(mouseState));
}

void input_process_key(keys key, b8 pressed) {
    if (key == KEY_LALT) {
        HINFO("Left alt %s.", (pressed ? "pressed" : "released"));
    }
    else if (key == KEY_RALT) {
        HINFO("Right alt %s.", (pressed ? "pressed" : "released"));
    }

    if (key == KEY_LCONTROL) {
        HINFO("Left ctrl %s.", (pressed ? "pressed" : "released"));
    }
    else if (key == KEY_RCONTROL) {
        HINFO("Right ctrl %s.", (pressed ? "pressed" : "released"));
    }

    if (key == KEY_LSHIFT) {
        HINFO("Left shift %s.", (pressed ? "pressed" : "released"));
    }
    else if (key == KEY_RSHIFT) {
        HINFO("Right shift %s.", (pressed ? "pressed" : "released"));
    }

    // Only handle this if the state was actually changed
    if (state_ptr->kcur.keys[key] != pressed) {
        // Update internal state
        state_ptr->kcur.keys[key] = pressed;

        // Fire an event for inmediate processing
        eventContext context;
        context.data.u16[0] = key;
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void input_process_button(buttons button, b8 pressed) {
    // If the state changed, fire an event
    if (state_ptr->mcur.buttons[button] != pressed) {
        state_ptr->mcur.buttons[button] = pressed;

        // Fire the event
        eventContext context;
        context.data.u16[0] = button;
        eventFire(pressed ? EVENT_CODE_MOUSE_BUTTON_PRESSED : EVENT_CODE_MOUSE_BUTTON_RELEASED, 0, context);
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    // Only proccess if actually different
    if (state_ptr->mcur.x != x || state_ptr->mcur.y != y) {
        // NOTE: Enable this if debugging
        // HDEBUG("Mouse pos: %i, %i.", x, y);

        // Update internal state
        state_ptr->mcur.x = x;
        state_ptr->mcur.y = y;

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
    if(!state_ptr) {
        return false;
    }
    return state_ptr->kcur.keys[key];
}

b8 input_key_up(keys key) {
    if(!state_ptr) {
        return false;
    }
    return !state_ptr->kcur.keys[key];
}

b8 input_was_key_down(keys key) {
    if(!state_ptr) {
        return false;
    }
    return state_ptr->kprev.keys[key];
}

b8 input_was_key_up(keys key) {
    if(!state_ptr) {
        return false;
    }
    return !state_ptr->kprev.keys[key];
}

b8 input_is_button_down(buttons button) {
    if(!state_ptr) {
        return false;
    }
    return state_ptr->mcur.buttons[button];
}

b8 input_is_button_up(buttons button) {
    if(!state_ptr) {
        return false;
    }
    return !state_ptr->mcur.buttons[button];
}

b8 input_was_button_down(buttons button) {
    if(!state_ptr) {
        return false;
    }
    return state_ptr->mprev.buttons[button];
}

b8 input_was_button_up(buttons button) {
    if(!state_ptr) {
        return false;
    }
    return !state_ptr->mprev.buttons[button];
}

void input_get_mouse_possition(i32* x, i32* y) {
    if(!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mcur.x;
    *y = state_ptr->mcur.y;
}

void input_get_previus_mouse_possition(i32* x, i32* y) {
    if(!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mprev.x;
    *y = state_ptr->mprev.y;
}