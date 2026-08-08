#include "font.h"
#include "draw.h"

static struct {
    /* camera currently being used to draw */
    draw_Camera camera;
    SDL_Surface *surface;
} draw;

void draw_text(char *str, int x, int y, uint32_t color, int scale) {
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

static void draw_os_window(void) {

    /* shadow */
    SDL_FillSurfaceRect(
        draw.surface,
        &(SDL_Rect) { 0, 2, draw.surface->w, DRAW_OS_WINDOW_THICKNESS },
        SDL_MapSurfaceRGBA(draw.surface, 0x08, 0x08, 0x08, 0xFF)
    );

    /* background */
    SDL_FillSurfaceRect(
        draw.surface,
        &(SDL_Rect) { 0, 0, draw.surface->w, DRAW_OS_WINDOW_THICKNESS },
        SDL_MapSurfaceRGBA(draw.surface, 0x33, 0x33, 0x33, 0xFF)
    );

    draw_text(
        "SpAT",
        5,
        6,
        SDL_MapSurfaceRGBA(draw.surface, 0xaa, 0xaa, 0xaa, 0xaa),
        2
    );
}

void draw_frame(SDL_Surface *surface, draw_Camera canvas_camera) {

    /* draw canvas content */
    {
        draw.surface = SDL_CreateSurfaceFrom(
            surface->w,
            surface->h - DRAW_OS_WINDOW_THICKNESS,
            surface->format,
            surface->pixels + surface->pitch * DRAW_OS_WINDOW_THICKNESS,
            surface->pitch
        );
        draw.camera = canvas_camera;

        Uint32 *pixels = (Uint32 *)draw.surface->pixels;
        int pitch = draw.surface->pitch / sizeof(Uint32);

        /* draw checkerboard */
        for (int y = 0; y < draw.surface->h; y++) {
            for (int x = 0; x < draw.surface->w; x++) {
                int cx = (x + draw.camera.x + 99999)%100;
                int cy = (y + draw.camera.y + 99999)%100;
                pixels[y * pitch + x] = (cx == 0 || cy == 0)
                    ? SDL_MapSurfaceRGBA(draw.surface, 0x44, 0x44, 0x44, 0xFF)
                    : SDL_MapSurfaceRGBA(draw.surface, 0x22, 0x22, 0x22, 0xFF)
                    ; 
            }
        }

        /* draw text */
        draw_text(
            "The five boxing wizards jump quickly",
            5,
            5,
            SDL_MapSurfaceRGBA(draw.surface, 0xff, 0xff, 0xff, 0xff),
            5
        );
        SDL_DestroySurface(draw.surface);
    }
    
    draw.surface = surface;
    draw.camera = (draw_Camera) {0};
    draw_os_window();
}
