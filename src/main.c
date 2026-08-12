#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "rect.h"
#include "canvas.h"
#include "os_window.h"
#include <math.h>

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Spaceport Arms Technician", "1.0", "com.problemchild.spat");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    {
        int size_x = 50 * 16;
        int size_y = 50 * 9;
        os_window_set_size(size_x, size_y);
        app.window = SDL_CreateWindow(
            "spat",
            size_x,
            size_y,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT
        );
        if (app.window == NULL) {
            SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // if (SDL_SetWindowSurfaceVSync(app.window, true))
        //     SDL_Log("vsync enabled");
        // else
        //     SDL_Log("vsync disabled: %s", SDL_GetError());
    }

    if (!SDL_SetWindowHitTest(app.window, os_window_hit_test, NULL)) {
        SDL_Log("Couldn't set hit test: %s", SDL_GetError());
    }

    app_cursors_init();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    if (os_window_event(event))
        return SDL_APP_CONTINUE;

    canvas_event(event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    SDL_Surface *sdl_surface = SDL_GetWindowSurface(app.window);
    if (sdl_surface == NULL) {
        SDL_Log("Couldn't get framebuffer surface: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    draw_Surface surface = {
        .extents = { .w = sdl_surface->w, .h = sdl_surface->h },
        .og_pixels = sdl_surface->pixels,
        .sdl_surface = sdl_surface,
    };

    {
        draw_Surface canvas_surface = surface;
        rect_cut_top(&canvas_surface.extents, os_window_TOP_BAR_THICKNESS);
        app.canvas_window_rect = canvas_surface.extents;

        draw_to(canvas_surface, canvas.camera);
        canvas_draw();
    }

    draw_to(surface, (draw_Camera) { .scale = 1 });
    os_window_set_size(sdl_surface->w, sdl_surface->h);
    os_window_fn(os_window_Mode_Draw, NULL);

    app_cursors_update();

    SDL_UpdateWindowSurface(app.window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    app_cursors_free();
}
