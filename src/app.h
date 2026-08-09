#ifndef __APP_HEADER
#define __APP_HEADER

#include <SDL3/SDL.h>

#define dbg() __builtin_debugtrap()
#define spat_min(x, y) (((x) < (y)) ? (x) : (y))
#define spat_max(x, y) (((x) > (y)) ? (x) : (y))

typedef struct {
    SDL_Window *window;
} App;

App app;

#endif
