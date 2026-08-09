#ifndef __APP_HEADER
#define __APP_HEADER

#include <SDL3/SDL.h>
#include "draw.h"

#define dbg() __builtin_debugtrap()
#define spat_min(x, y) (((x) < (y)) ? (x) : (y))
#define spat_max(x, y) (((x) > (y)) ? (x) : (y))

typedef struct {
    SDL_Window *window;

    struct {
        int canvas_mouse_down_x, canvas_mouse_down_y;
        int camera_mouse_down_x, camera_mouse_down_y;

        int canvas_mouse_x, canvas_mouse_y;
        int window_mouse_x, window_mouse_y;

        bool canvas_mouse_down;

        /* window size */
        int size_x, size_y;
    } input;

    draw_Camera camera;

} App;

App app;

#endif
