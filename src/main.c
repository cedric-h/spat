#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define dbg() __builtin_debugtrap()
#define spat_min(x, y) (((x) < (y)) ? (x) : (y))
#define spat_max(x, y) (((x) > (y)) ? (x) : (y))

#include "draw.h"
#include <math.h>

static struct {
    SDL_Window *window;

    struct {
        int mouse_down_x, mouse_down_y;
        int camera_mouse_down_x, camera_mouse_down_y;
        int mouse_x, mouse_y;
        bool mouse_down;
    } input;

    draw_Camera camera;

} app = {0};

SDL_HitTestResult hit_test(SDL_Window *win, const SDL_Point *p, void *data) {
    if (p->y < DRAW_OS_WINDOW_THICKNESS)
        return SDL_HITTEST_DRAGGABLE;

    return SDL_HITTEST_NORMAL;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Space Port Arms Technician", "1.0", "com.problemchild.spat");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    app.window = SDL_CreateWindow(
        "spat", 50*16, 50*9,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT
    );
    if (app.window == NULL) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetWindowHitTest(app.window, hit_test, NULL)) {
        SDL_Log("Couldn't set hit test: %s", SDL_GetError());
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: {
            return SDL_APP_SUCCESS;
        } break;

        case SDL_EVENT_MOUSE_MOTION: {
            app.input.mouse_x = event->motion.x;
            app.input.mouse_y = event->motion.y;

            if (app.input.mouse_down) {
                app.camera.x = app.input.camera_mouse_down_x
                    + (app.input.mouse_down_x - app.input.mouse_x);
                app.camera.y = app.input.camera_mouse_down_y
                    + (app.input.mouse_down_y - app.input.mouse_y);
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            app.input.mouse_down = true;
            app.input.mouse_down_x = event->button.x;
            app.input.mouse_down_y = event->button.y;
            app.input.camera_mouse_down_x = app.camera.x;
            app.input.camera_mouse_down_y = app.camera.y;
        } break;

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            app.input.mouse_down = false;
        } break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    SDL_Surface *surface = SDL_GetWindowSurface(app.window);
    if (surface == NULL) {
        SDL_Log("Couldn't get framebuffer surface: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    draw_frame(surface, app.camera);

    SDL_UpdateWindowSurface(app.window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}
