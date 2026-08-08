#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Space Port Arms Technician", "1.0", "com.problemchild.spat");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow(
        "spat", 640, 480,
        SDL_WINDOW_RESIZABLE
    );
    if (window == NULL) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pitch = surface->pitch / sizeof(Uint32);

    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            Uint8 r = (Uint8)(x * 255 / surface->w);
            Uint8 g = (Uint8)(y * 255 / surface->h);
            Uint8 b = 128;

            pixels[y * pitch + x] =
                SDL_MapSurfaceRGB(surface, r, g, b);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}
