#ifndef __DRAW_HEADER
#define __DRAW_HEADER

#define DRAW_OS_WINDOW_THICKNESS 25
#include <SDL3/SDL.h>

typedef struct {
    int x, y, scale;
} draw_Camera;

void draw_to(SDL_Surface *surface, draw_Camera canvas_camera);
void draw_background(void);

void draw_text(char *str, int x, int y, uint32_t color, int scale);
void draw_rect_outline(SDL_Rect rect, uint32_t color);
void draw_rect(SDL_Rect rect, uint32_t color);
void draw_px(int x, int y, uint32_t color);

uint32_t draw_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);


#endif
