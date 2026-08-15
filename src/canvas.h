#ifndef __CANVAS_HEADER
#define __CANVAS_HEADER
#include "draw.h"
#include <SDL3/SDL.h>

bool canvas_event(SDL_Event *event);
void canvas_draw(void);

typedef struct {
    struct {
        /* all of these are in screenspace, not worldspace (see canvas.mouse for worldspace) */
        SDL_Point mouse_down, camera_mouse_down, mouse;
        bool is_mouse_down;
    } input;

    /* mouse in world position (mouse position in input is in screen position) */
    SDL_Point mouse;

    draw_Camera camera;
} canvas_Canvas;

canvas_Canvas canvas;

#endif
