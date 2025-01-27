#include "core/app.h"
#include "game_types.h"

#include "core/logger.h"

#include "platform/platform.h"
#include "memory/hmemory.h"
#include "core/events.h"
#include "core/input.h"
#include "core/clock.h"

#include "memory/linear_allocator.h"

#include "renderer/frontend.h"

typedef struct appState {
    game* gameInstance;
    b8 isRunning;
    b8 isSuspended;
    platformState platform;
    i16 width, height;
    
    hclock clock;
    f64 lastTime;

    linear_allocator systems_allocator;

    u64 event_system_memory_requirement;
    void* event_system_state;

    u64 memory_system_memory_requirement;
    void* memory_system_state;

    u64 logging_system_memory_requirement;
    void* logging_system_state;

    u64 input_system_memory_requirement;
    void* input_system_state;

    u64 platform_system_memory_requirement;
    void* platform_system_state;

    u64 renderer_system_memory_requirement;
    void* renderer_system_state;
} appState;

static appState* app;

// Event handlers
b8 appOnEvent(u16 code, void* sender, void* listenerInstance, eventContext context);
b8 appOnKey(u16 code, void* sender, void* listenerInstance, eventContext context);
b8 appOnResized(u16 code, void* sender, void* listenerInstance, eventContext context);

b8 appCreate(game* gameInstance) {
    if(gameInstance->appState) {
        HERROR("appCreate called more than once");
        return false;
    }

    // Allocate memory for app state
    gameInstance->appState = Hallocate(sizeof(appState), MEMORY_TAG_APPLICATION);
    app = gameInstance->appState;

    app->gameInstance = gameInstance;
    app->isRunning = false;
    app->isSuspended = false;

    u64 sysAllocTotalSize = 64 * 1024 * 1024; // 64 megabytes.
    create_linear_allocator(sysAllocTotalSize, NULL, &app->systems_allocator);

    // Initialize subsystems
    // Events subsystem
    eventInit(&app->event_system_memory_requirement, NULL);
    app->event_system_state = allocate_linear_allocator(&app->systems_allocator, app->event_system_memory_requirement);
    eventInit(&app->event_system_memory_requirement, app->event_system_state);

    // Memory subsystem
    initializeMemory(&app->memory_system_memory_requirement, NULL);
    app->memory_system_state = allocate_linear_allocator(&app->systems_allocator, app->memory_system_memory_requirement);
    initializeMemory(&app->memory_system_memory_requirement, app->memory_system_state);

    // Logging subsystem
    initLog(&app->logging_system_memory_requirement, NULL);
    app->logging_system_state = allocate_linear_allocator(&app->systems_allocator, app->logging_system_memory_requirement);
    if (!initLog(&app->logging_system_memory_requirement, app->logging_system_state)) {
        HERROR("Failed to initialize logging system, shutting down...");
        return false;
    }
    
    // Input subsystem
    inputInit(&app->input_system_memory_requirement, NULL);
    app->input_system_state = allocate_linear_allocator(&app->systems_allocator, app->input_system_memory_requirement);
    inputInit(&app->input_system_memory_requirement, app->input_system_state);

    // Register for engine-level events
    eventRegister(EVENT_CODE_APPLICATION_QUIT, NULL, appOnEvent);
    eventRegister(EVENT_CODE_KEY_PRESSED, NULL, appOnKey);
    eventRegister(EVENT_CODE_KEY_RELEASED, NULL, appOnKey);
    eventRegister(EVENT_CODE_RESIZED, NULL, appOnResized);

    // Platform subsystem
    platformStartup(&app->platform_system_memory_requirement, NULL, NULL, 0, 0, 0, 0);
    app->platform_system_state = allocate_linear_allocator(&app->systems_allocator, app->platform_system_memory_requirement);
    if (!platformStartup(
        &app->platform_system_memory_requirement,
        app->platform_system_state,
        gameInstance->config.name,
        gameInstance->config.startPosX, 
        gameInstance->config.startPosY, 
        gameInstance->config.startWidth, 
        gameInstance->config.startHeight)) {
        
        return false;
    }

    // Renderer subsystem
    initRenderer(&app->renderer_system_memory_requirement, NULL, NULL);
    app->renderer_system_state = allocate_linear_allocator(&app->systems_allocator, app->renderer_system_memory_requirement);
    if (!initRenderer(&app->renderer_system_memory_requirement, app->renderer_system_state, gameInstance->config.name)) {
        HFATAL("Failed to initialize renderer. Aborting application.");
        return false;
    }

    // Initialize the game
    if(!app->gameInstance->initialize(app->gameInstance)) {
        HFATAL("Game failed to initialize.");
        return false;
    }

    // Call resize once to ensure the proper size has been set.
    app->gameInstance->onResize(app->gameInstance, app->width, app->height);

    return true;
}

b8 appRun() {
    app->isRunning = true;
    startClock(&app->clock);
    updateClock(&app->clock);
    app->lastTime = app->clock.elapsed;

    f64 running_time = 0;
    f64 frame_count = 0;
    f64 target_framerate = 1.0f / 60;

    HINFO(GetMemoryUsage_str());

    while (app->isRunning) {
        if (!platformPumpMessages()) {
            app->isRunning = false;
        }
        if(!app->isSuspended) {
            // Update clock and get delta time.
            updateClock(&app->clock);
            f64 curTime = app->clock.elapsed;
            f64 delta = (curTime - app->lastTime);
            f64 frame_start_time = platformGetAbsoluteTime();

            if(!app->gameInstance->update(app->gameInstance, (f32)delta)) {
                HFATAL("Game update failed, shutting down...");
                app->isRunning = false;
                break;
            }

            //Call the game's render routine
            if(!app->gameInstance->render(app->gameInstance, (f32)delta)) {
                HFATAL("Game update failed, shutting down...");
                app->isRunning = false;
                break;
            }

            // TODO: Change packet generation to renderer
            renderPacket packet;
            packet.delta_time = delta;
            rendererDrawFrame(&packet);

            // Figure out how long the frame took
            f64 frame_end_time = platformGetAbsoluteTime();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_seconds = target_framerate - frame_elapsed_time;

            if(remaining_seconds > 0) {
                u64 remaining_ms = (remaining_seconds * 1000);

                // If there is time left, give it back to the OS.
                b8 limit_frames = false;
                if(remaining_ms > 0 && limit_frames) {
                    platformSleep(remaining_ms - 1);
                }

                frame_count++;
            }

            // NOTE: Input update should always be handled
            // after any input should be recorded.
            // As a safety measure, input is the last thing
            // to be updated before the current frame ends.
            inputUpdate(delta);

            app->lastTime = curTime;
        }
    }

    app->isRunning = false;

    eventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, appOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESSED, 0, appOnKey);
    eventUnregister(EVENT_CODE_KEY_RELEASED, 0, appOnKey);

    inputShutdown(app->input_system_state);

    shutdownRenderer(app->renderer_system_state);

    platformShutdown(app->platform_system_state);

    shutdownMemory(app->memory_system_state);

    eventShutdown(app->event_system_state);
    
    return true;
}

void appGetFramebufferSize(u32* width, u32* height) {
    *width = app->width;
    *height = app->height;
}

b8 appOnEvent(u16 code, void* sender, void* listenerInstance, eventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            HINFO("EVENT_CODE_APPLICATION_QUIT recived, shutting down");
            app->isRunning = false;
            return true;
        }  
    }

    return false;
}

b8 appOnKey(u16 code, void* sender, void* listenerInstance, eventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 keyCode = context.data.u16[0];

        // Exit app when hitting escape key
        /*if (keyCode == KEY_ESCAPE) {
            // NOTE: Technically firing an event to itself, but there may be other listeners
            eventContext data = {};
            eventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            // Block anything else from processing this
            return true;
        }*/

        /*if (keyCode == KEY_A) {
            // Example on checking for a key
            HDEBUG("Explicit key A was pressed");
        }*/
        HDEBUG("Key %c was pressed", keyCode);
    }
    else if(code == EVENT_CODE_KEY_RELEASED) {
        u16 keyCode = context.data.u16[0];

        /*if (keyCode == KEY_A) {
            // Example on checking for a key
            HDEBUG("Explicit key A was released");
        }*/
        HDEBUG("Key %c was released", keyCode);
    }
    return false;
}

b8 appOnResized(u16 code, void* sender, void* listenerInstance, eventContext context) {
    if (code == EVENT_CODE_RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // Check if different. If so, trigger a resize event.
        if (width != app->width || height != app->height) {
            app->width = width;
            app->height = height;

            HDEBUG("Window resize: %i, %i", width, height);

            // Handle minimization // TODO: this will be optional
            if (width == 0 || height == 0) {
                HINFO("Window minimized, suspending application");
                app->isSuspended = true;
                return true;
            }
            else {
                if (app->isSuspended) {
                    HINFO("Window restored, resuming application");
                    app->isSuspended = false;
                }
                app->gameInstance->onResize(app->gameInstance, width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this.
    return false;
}