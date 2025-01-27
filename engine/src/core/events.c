#include "core/events.h"

#include "memory/hmemory.h"
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

// *Event system internal state pointer*
static eventSystemState* state_ptr;

void eventInit(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(eventSystemState);
    if (state == NULL) {
        return;
    }
    HzeroMemory(state, sizeof(state));
    state_ptr = state;
}

void eventShutdown(void* state) {
    // Free the events arrays. And objects pointed to should be destroyed on their own.
    if (state_ptr) {
        for (u16 i = 0; i < MAX_MESSAGE_CODES; i++) {
            if (state_ptr->registered[i] != NULL && state_ptr->registered[i]->events != 0) {
                darray_destroy(state_ptr->registered[i]->events);
                state_ptr->registered[i]->events = 0;
                Hfree(state_ptr->registered[i], sizeof(eventCodeEntry), MEMORY_TAG_EVENT);
                state_ptr->registered[i] = NULL;
            }
        }
    }
    state_ptr = NULL;
}


b8 eventRegister(u16 code, void* listener, PFNC_onEvent onEvent) {
    if (!state_ptr) {
        return false;
    }

    if (state_ptr->registered[code] == NULL) { 
        state_ptr->registered[code] = (eventCodeEntry*)Hallocate(sizeof(eventCodeEntry), MEMORY_TAG_EVENT); 
        if (state_ptr->registered[code] == NULL) {
            return false; 
        }
        state_ptr->registered[code]->events = NULL; 
    }

    if (!state_ptr->registered[code] || !state_ptr->registered[code]->events) {
       state_ptr->registered[code]->events = darray_create(registeredEvent);
    }

    u64 registeredCount = darray_length(state_ptr->registered[code]->events);
    for(u64 i = 0; i < registeredCount; i++) {
        if (state_ptr->registered[code]->events[i].listener == listener) {
            // TODO: WARNING
            return false;
        }
    }

    // If at this point no duplicate was found. Proceed with registration.
    registeredEvent event;
    event.listener = listener;
    event.callback = onEvent;
    darray_push(state_ptr->registered[code]->events, event);

    return true;
}

b8 eventUnregister(u16 code, void* listener, PFNC_onEvent onEvent) {
    if (!state_ptr) {
        return false;
    }

    // If nothing is registered, boot out.
    if (state_ptr->registered[code]->events == NULL) {
        // TODO: WARNING
        return false;
    }

    u64 registeredCount = darray_length(state_ptr->registered[code]->events);
    for(u64 i = 0; i < registeredCount; i++) {
        registeredEvent e = state_ptr->registered[code]->events[i];
        if (e.listener == listener && e.callback == onEvent) {
            // Found one, remove it
            registeredEvent popped;
            darray_pop_at(state_ptr->registered[code]->events, i, &popped);
            return true;
        }
    }

    // Not found
    return false;
}

b8 eventFire(u16 code, void* sender, eventContext context) {
    if (!state_ptr) {
        return false;
    }

    // If nothing is registered for this event code, boot out
    if (!state_ptr->registered[code] || !state_ptr->registered[code]->events) {
        return false; 
    }

    u64 registeredCount = darray_length(state_ptr->registered[code]->events);
    for(u64 i = 0; i < registeredCount; i++) {
        registeredEvent e = state_ptr->registered[code]->events[i];
        if (e.callback(code, sender, e.listener, context)) {
            // Event has been handled, do not send to other listeners
            return true;
        }
    }
    // Not found
    return false;
}