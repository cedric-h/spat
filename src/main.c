#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "draw.h"
#include "os_window.h"
#include <math.h>

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Space Port Arms Technician", "1.0", "com.problemchild.spat");

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

        if (SDL_SetWindowSurfaceVSync(app.window, true))
            SDL_Log("vsync enabled");
        else
            SDL_Log("vsync disabled: %s", SDL_GetError());
    }

    if (!SDL_SetWindowHitTest(app.window, os_window_hit_test, NULL)) {
        SDL_Log("Couldn't set hit test: %s", SDL_GetError());
    }

    return SDL_APP_CONTINUE;
}

bool canvas_event(SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: {
            return SDL_APP_SUCCESS;
        } break;

        case SDL_EVENT_MOUSE_MOTION: {
            app.input.canvas_mouse_x = event->motion.x;
            app.input.canvas_mouse_y = event->motion.y;

            if (app.input.canvas_mouse_down) {
                app.camera.x = app.input.camera_mouse_down_x
                    + (app.input.canvas_mouse_down_x - app.input.canvas_mouse_x);
                app.camera.y = app.input.camera_mouse_down_y
                    + (app.input.canvas_mouse_down_y - app.input.canvas_mouse_y);
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            app.input.canvas_mouse_down = true;
            app.input.canvas_mouse_down_x = event->button.x;
            app.input.canvas_mouse_down_y = event->button.y;
            app.input.camera_mouse_down_x = app.camera.x;
            app.input.camera_mouse_down_y = app.camera.y;
        } break;

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            app.input.canvas_mouse_down = false;
        } break;
    }

    return false;
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
    SDL_Surface *surface = SDL_GetWindowSurface(app.window);
    if (surface == NULL) {
        SDL_Log("Couldn't get framebuffer surface: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    os_window_set_size(surface->w, surface->h);

    {
        SDL_Surface *canvas = SDL_CreateSurfaceFrom(
            surface->w,
            surface->h - DRAW_OS_WINDOW_THICKNESS,
            surface->format,
            surface->pixels + surface->pitch * DRAW_OS_WINDOW_THICKNESS,
            surface->pitch
        );
        draw_to(canvas, app.camera);
        draw_background();

        {
            draw_text(
                "The five boxing wizards jump quickly",
                5,
                5,
                SDL_MapSurfaceRGBA(draw.surface, 0xff, 0xff, 0xff, 0xff),
                1
            );

            uint32_t in  = SDL_MapSurfaceRGBA(draw.surface, 0xff, 0x00, 0xff, 0xff);
            uint32_t out = SDL_MapSurfaceRGBA(draw.surface, 0xff, 0xff, 0xff, 0xff);
            draw_rect((SDL_Rect) { 0, 0, 100, 100 }, in);
            draw_rect_outline((SDL_Rect) { 0, 0, 100, 100 }, out);

            draw_rect((SDL_Rect) { 100, 100, 100, 100 }, in);
            draw_rect_outline((SDL_Rect) { 100, 100, 100, 100 }, out);

            draw_rect((SDL_Rect) { 200, 200, 100, 100 }, in);
            draw_rect_outline((SDL_Rect) { 200, 200, 100, 100 }, out);
        }

        SDL_DestroySurface(canvas);
    }

    draw.surface = surface;
    draw.camera = (draw_Camera) { .scale = 1 };

    os_window_fn(os_window_Mode_Draw, NULL);

    SDL_UpdateWindowSurface(app.window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}
