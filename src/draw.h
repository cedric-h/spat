#ifndef __DRAW_HEADER
#define __DRAW_HEADER

#include <SDL3/SDL.h>

#define draw_clr_bg     draw_rgba(0x33, 0x33, 0x33, 0xFF)
#define draw_clr_shadow draw_rgba(0x08, 0x08, 0x08, 0xFF)
#define draw_clr_text   draw_rgba(0x66, 0x66, 0x66, 0xFF)
#define draw_clr_black  draw_rgba(0x11, 0x11, 0x11, 0xFF)
#define draw_clr_hilite draw_rgba(0xFF, 0xAA, 0xAA, 0xFF)

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
