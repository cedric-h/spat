#include "font.h"
#include "draw.h"
#include "rect.h"
#include "app.h" /* app.input */

static struct {
    /* camera currently being used to draw */
    draw_Camera camera;
    SDL_Surface *surface;
} draw;

SDL_Rect draw_text_measure(char *str, int x, int y, int scale) {
    SDL_Rect r = { .x = x, .y = y, .h = 8*scale };
    for (; *str; str++) r.w += 6*scale;
    r.w -= 1*scale;
    return r;
}
void draw_text_centered(char *str, SDL_Rect r, uint32_t color, int scale) {
    SDL_Rect center = rect_centered_in(r, draw_text_measure(str, r.x, r.y, scale));
    draw_text(str, center.x, center.y, color, scale);
}

void draw_text(char *str, int x, int y, uint32_t color, int scale) {
    x *= draw.camera.scale;
    y *= draw.camera.scale;
    scale *= draw.camera.scale;

    Uint32 *pixels = (Uint32 *)draw.surface->pixels;
    int pitch = draw.surface->pitch / sizeof(Uint32);
    int dst_x = x - draw.camera.x - 3*scale;
    int dst_y = y - draw.camera.y;

    for (; *str; str++) {
        for (int y = 0; y < 8*scale; y++) {
            if ((dst_y + y) > draw.surface->h) break;
            if ((dst_y + y) < 0) continue;

            char row = font_data[((size_t)*str)*8 + y/scale];
            for (int x = 0; x < 8*scale; x++) {
                if ((dst_x + x) >= draw.surface->w) break;
                if ((dst_x + x) < 0) continue;

                bool lit = (row & (1 << (8 - x/scale))) > 0;
                if (!lit) continue;

                pixels[(dst_y + y)*pitch + (dst_x + x)] = color;
            }
        }

        dst_x += 6*scale;
    }
}

void draw_px(int x, int y, uint32_t color) {
    if (x < 0) return;
    if (y < 0) return;
    if (x >= draw.surface->w) return;
    if (y >= draw.surface->h) return;

    Uint32 *pixels = (Uint32 *)draw.surface->pixels;
    int pitch = draw.surface->pitch / sizeof(Uint32);
    pixels[y*pitch + x] = color;
}

void draw_rect(SDL_Rect rect, uint32_t color) {
    rect.x *= draw.camera.scale;
    rect.y *= draw.camera.scale;
    rect.w *= draw.camera.scale;
    rect.h *= draw.camera.scale;
    rect.x -= draw.camera.x;
    rect.y -= draw.camera.y;

    SDL_FillSurfaceRect(draw.surface, &rect, color);
}

void draw_rect_outline(SDL_Rect rect, uint32_t color) {
    rect.x *= draw.camera.scale;
    rect.y *= draw.camera.scale;
    rect.w *= draw.camera.scale;
    rect.h *= draw.camera.scale;
    rect.x -= draw.camera.x;
    rect.y -= draw.camera.y;
    int w = rect.w, h = rect.h;

    for (int x = 0; x <= w; x++) draw_px(rect.x + x, rect.y + 0, color);
    for (int x = 0; x <= w; x++) draw_px(rect.x + x, rect.y + h, color);
    for (int y = 0; y <= h; y++) draw_px(rect.x + 0, rect.y + y, color);
    for (int y = 0; y <= h; y++) draw_px(rect.x + w, rect.y + y, color);
}

void draw_to(draw_Surface draw_surface, draw_Camera camera) {
    SDL_Surface *s = draw_surface.sdl_surface;
    s->w = draw_surface.extents.w;
    s->h = draw_surface.extents.h;
    s->pixels = draw_surface.og_pixels +
        s->pitch * draw_surface.extents.y +
        draw_surface.extents.x;

    draw.surface = s;
    draw.camera = camera;
}

void draw_background(void) {
    Uint32 *pixels = (Uint32 *)draw.surface->pixels;
    int pitch = draw.surface->pitch / sizeof(Uint32);

    /* draw grid */
    for (int y = 0; y < draw.surface->h; y++) {
        for (int x = 0; x < draw.surface->w; x++) {
            int cx = SDL_abs(x + draw.camera.x)%100;
            int cy = SDL_abs(y + draw.camera.y)%100;
            pixels[y * pitch + x] = (cx == 0 || cy == 0)
                ? SDL_MapSurfaceRGBA(draw.surface, 0x44, 0x44, 0x44, 0xFF)
                : SDL_MapSurfaceRGBA(draw.surface, 0x22, 0x22, 0x22, 0xFF)
                ; 
        }
    }
}

uint32_t draw_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return SDL_MapSurfaceRGBA(draw.surface, r, g, b, a);
}
