#include "core/app.h"
#include "game_types.h"

#include "core/logger.h"

#include "platform/platform.h"
#include "core/hmemory.h"
#include "core/events.h"
#include "core/input.h"
#include "core/clock.h"

#include "renderer/frontend.h"

typedef struct appState {
    game* gameInstance;
    b8 isRunning;
    b8 isSuspended;
    platformState platform;
    i16 width, height;
    clock clock;
    f64 lastTime;
} appState;

static b8 initialized = false;
static appState app;

// Event handlers
b8 appOnEvent(u16 code, void* sender, void* listenerInstance, eventContext context);
b8 appOnKey(u16 code, void* sender, void* listenerInstance, eventContext context);
b8 appOnResized(u16 code, void* sender, void* listenerInstance, eventContext context);

b8 appCreate(game* gameInstance) {
    if(initialized) {
        HERROR("appCreate called more than once");
        return false;
    }

    app.gameInstance = gameInstance;

    // Initialize subsystems
    initLog();
    inputInit();

    app.isRunning = true;
    app.isSuspended = false;

    if(!eventInit()) {
        HERROR("Event system falied initialization. Application cannot continue.");
        return false;
    }

    eventRegister(EVENT_CODE_APPLICATION_QUIT, NULL, appOnEvent);
    eventRegister(EVENT_CODE_KEY_PRESSED, NULL, appOnKey);
    eventRegister(EVENT_CODE_KEY_RELEASED, NULL, appOnKey);
    eventRegister(EVENT_CODE_RESIZED, NULL, appOnResized);

    if(!platformStartup(
        &app.platform,
        gameInstance->config.name,
        gameInstance->config.startPosX, 
        gameInstance->config.startPosY, 
        gameInstance->config.startWidth, 
        gameInstance->config.startHeight)) {
            return false;
    }

    // Renderer strartup
    if (!initRenderer(gameInstance->config.name, &app.platform)) {
        HFATAL("Failed to initialize renderer. Aborting application");
        return false;
    }

    // Initialize the game
    if(!app.gameInstance->initialize(app.gameInstance)) {
        HFATAL("Game failed to initialize");
        return false;
    }

    app.gameInstance->onResize(app.gameInstance, app.width, app.height);

    initialized = true;

    return true;
}

b8 appRun() {
    startClock(&app.clock);
    updateClock(&app.clock);
    app.lastTime = app.clock.elapsed;

    f64 running_time = 0;
    f64 frame_count = 0;
    f64 target_framerate = 1.0f / 60;

    HINFO(GetMemoryUsage_str());

    while (app.isRunning) {
        if(!platformPumpMessages(&app.platform)) {
            app.isRunning = false;
        }
        if(!app.isSuspended) {
            // Update clock and get delta time.
            updateClock(&app.clock);
            f64 curTime = app.clock.elapsed;
            f64 delta = (curTime - app.lastTime);
            f64 frame_start_time = platformGetAbsoluteTime();

            if(!app.gameInstance->update(app.gameInstance, (f32)delta)) {
                HFATAL("Game update failed, shutting down...");
                app.isRunning = false;
                break;
            }

            //Call the game's render routine
            if(!app.gameInstance->render(app.gameInstance, (f32)delta)) {
                HFATAL("Game update failed, shutting down...");
                app.isRunning = false;
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

            app.lastTime = curTime;

            // TODO: Delete this
            if(running_time > 0 && frame_count > 0) {}
        }
    }

    app.isRunning = false;

    eventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, appOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESSED, 0, appOnKey);
    eventUnregister(EVENT_CODE_KEY_RELEASED, 0, appOnKey);

    eventShutdown();
    shutdownRenderer();
    platformShutdown(&app.platform);

    return true;
}

void appGetFramebufferSize(u32* width, u32* height) {
    *width = app.width;
    *height = app.height;
}

b8 appOnEvent(u16 code, void* sender, void* listenerInstance, eventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            HINFO("EVENT_CODE_APPLICATION_QUIT recived, shutting down");
            app.isRunning = false;
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
        if (width != app.width || height != app.height) {
            app.width = width;
            app.height = height;

            HDEBUG("Window resize: %i, %i", width, height);

            // Handle minimization // TODO: this will be optional
            if (width == 0 || height == 0) {
                HINFO("Window minimized, suspending application");
                app.isSuspended = true;
                return true;
            }
            else {
                if (app.isSuspended) {
                    HINFO("Window restored, resuming application");
                    app.isSuspended = false;
                }
                app.gameInstance->onResize(app.gameInstance, width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this.
    return false;
}