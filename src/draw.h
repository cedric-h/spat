#ifndef __DRAW_HEADER
#define __DRAW_HEADER

#define DRAW_OS_WINDOW_THICKNESS 25
#include <SDL3/SDL.h>

typedef struct {
    int x, y;
} draw_Camera;

void draw_text(char *str, int x, int y, uint32_t color, int scale);
void draw_frame(SDL_Surface *surface, draw_Camera canvas_camera);

#endif
