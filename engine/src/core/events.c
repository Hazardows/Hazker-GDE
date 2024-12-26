#include "core/events.h"

#include "core/hmemory.h"
#include "containers/darray.h"

typedef struct registeredEvent {
    void* listener;
    PFNC_onEvent callback;
} registeredEvent;

typedef struct eventCodeEntry {
    registeredEvent* events;
} eventCodeEntry;

// This should be more than enough codes...
#define MAX_MESSAGE_CODES 16384

// State structure.
typedef struct eventSystemState {
    // Lookup table for event codes.
    eventCodeEntry* registered[MAX_MESSAGE_CODES];
} eventSystemState;

// *Event system internal state*
static b8 isInitialized = false;
static eventSystemState state;

b8 eventInit() {
    if (isInitialized == true) {
        return false;
    }
    isInitialized = false;
    HzeroMemory(&state, sizeof(state));

    isInitialized = true;
    return true;
}

void eventShutdown() {
    // Free the events arrays. And objects pointed to should be destroyed on their own.
    for (u16 i = 0; i < MAX_MESSAGE_CODES; i++) {
        if (state.registered[i] != NULL && state.registered[i]->events != 0) {
            darray_destroy(state.registered[i]->events);
            state.registered[i]->events = 0;
            Hfree(state.registered[i], sizeof(eventCodeEntry), MEMORY_TAG_EVENT);
            state.registered[i] = NULL;
        }
    }
}


b8 eventRegister(u16 code, void* listener, PFNC_onEvent onEvent) {
    if (isInitialized == false) {
        return false;
    }

    if (state.registered[code] == NULL) { 
        state.registered[code] = (eventCodeEntry*)Hallocate(sizeof(eventCodeEntry), MEMORY_TAG_EVENT); 
        if (state.registered[code] == NULL) {
            return false; 
        }
        state.registered[code]->events = NULL; 
    }

    if (!state.registered[code] || !state.registered[code]->events) {
       state.registered[code]->events = darray_create(registeredEvent);
    }

    u64 registeredCount = darray_length(state.registered[code]->events);
    for(u64 i = 0; i < registeredCount; i++) {
        if (state.registered[code]->events[i].listener == listener) {
            // TODO: WARNING
            return false;
        }
    }

    // If at this point no duplicate was found. Proceed with registration.
    registeredEvent event;
    event.listener = listener;
    event.callback = onEvent;
    darray_push(state.registered[code]->events, event);

    return true;
}

b8 eventUnregister(u16 code, void* listener, PFNC_onEvent onEvent) {
    if (isInitialized == false) {
        return false;
    }

    // If nothing is registered, boot out.
    if (state.registered[code]->events == 0) {
        // TODO: WARNING
        return false;
    }

    u64 registeredCount = darray_length(state.registered[code]->events);
    for(u64 i = 0; i < registeredCount; i++) {
        registeredEvent e = state.registered[code]->events[i];
        if (e.listener == listener && e.callback == onEvent) {
            // Found one, remove it
            registeredEvent popped;
            darray_pop_at(state.registered[code]->events, i, &popped);
            return true;
        }
    }

    // Not found
    return false;
}

b8 eventFire(u16 code, void* sender, eventContext context) {
    if (!isInitialized) {
        return false;
    }

    // If nothing is registered for this event code, boot out
    if (!state.registered[code] || !state.registered[code]->events) {
        return false; 
    }

    u64 registeredCount = darray_length(state.registered[code]->events);
    for(u64 i = 0; i < registeredCount; i++) {
        registeredEvent e = state.registered[code]->events[i];
        if (e.callback(code, sender, e.listener, context)) {
            // Event has been handled, do not send to other listeners
            return true;
        }
    }

    // Not found
    return false;
}