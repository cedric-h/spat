#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define dbg() __builtin_debugtrap()
#define spat_min(x, y) (((x) < (y)) ? (x) : (y))
#define spat_max(x, y) (((x) > (y)) ? (x) : (y))

#include "font.h"
#include <math.h>

static struct {
    SDL_Window *window;

    struct {
        int x, y;
        int mouse_down_x, mouse_down_y;
    } camera;

    struct {
        int mouse_down_x, mouse_down_y;
        int mouse_x, mouse_y;
        bool mouse_down;
    } input;

} app = {0};

SDL_HitTestResult hit_test(SDL_Window *win, const SDL_Point *area, void *data) {
    // return SDL_HITTEST_DRAGGABLE;
    return SDL_HITTEST_NORMAL;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Space Port Arms Technician", "1.0", "com.problemchild.spat");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    app.window = SDL_CreateWindow(
        "spat", 640, 480,
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
                app.camera.x = app.camera.mouse_down_x
                    + (app.input.mouse_down_x - app.input.mouse_x);
                app.camera.y = app.camera.mouse_down_y
                    + (app.input.mouse_down_y - app.input.mouse_y);
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            app.input.mouse_down = true;
            app.input.mouse_down_x = event->button.x;
            app.input.mouse_down_y = event->button.y;
            app.camera.mouse_down_x = app.camera.x;
            app.camera.mouse_down_y = app.camera.y;
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

    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pitch = surface->pitch / sizeof(Uint32);

    /* draw checkerboard */
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            pixels[y * pitch + x] = (((x/10) ^ (y/10))&2)
                ? SDL_MapSurfaceRGBA(surface, 0x44, 0x44, 0x44, 0xFF)
                : SDL_MapSurfaceRGBA(surface, 0x22, 0x22, 0x22, 0xFF)
                ; 
        }
    }

    /* DEBUG: draw mouse */
    if (0) {
        int wx = (uint32_t)spat_max(0, spat_min((int)surface->w-1, app.input.mouse_x));
        int wy = (uint32_t)spat_max(0, spat_min((int)surface->h-1, app.input.mouse_y));
        for (int y = wy-4; y <= (wy+4); y++)
            for (int x = wx-4; x <= (wx+4); x++)
                pixels[y * pitch + x] =
                    SDL_MapSurfaceRGBA(surface, 0xFF, 0xFF, 0xFF, 0xFF);
    }

    /* draw text */
    {
        int dst_x = 15 - app.camera.x;
        int dst_y = 15 - app.camera.y;

        int scale = 5;
        for (char *str = "The five boxing wizards jump quickly"; *str; str++) {
            for (int y = 0; y < 8*scale; y++) {
                if ((dst_y + y) > surface->h) break;
                if ((dst_y + y) < 0) continue;

                char row = font_data[((size_t)*str)*8 + y/scale];
                for (int x = 0; x < 8*scale; x++) {
                    if ((dst_x + x) >= surface->w) break;
                    if ((dst_x + x) < 0) continue;

                    bool lit = (row & (1 << (8 - x/scale))) > 0;
                    if (!lit) continue;

                    pixels[(dst_y + y)*pitch + (dst_x + x)] =
                        SDL_MapSurfaceRGBA(surface, 0xff, 0xff, 0xff, 0xff);
                }
            }

            dst_x += 6*scale + 8*sinf(dst_y * 0.015f);
            dst_y += 18*sinf(dst_x * 0.015f);
        }
    }

    SDL_UpdateWindowSurface(app.window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}
