#include "canvas.h"
#include "draw.h"

static struct {
    struct {
        int mouse_down_x, mouse_down_y;
        int camera_mouse_down_x, camera_mouse_down_y;

        int mouse_x, mouse_y;

        bool mouse_down;
    } input;

    draw_Camera camera;
} canvas = {
    .camera.scale = 2,
};

bool canvas_event(SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: {
            return SDL_APP_SUCCESS;
        } break;

        case SDL_EVENT_MOUSE_MOTION: {
            canvas.input.mouse_x = event->motion.x;
            canvas.input.mouse_y = event->motion.y;

            if (canvas.input.mouse_down) {
                canvas.camera.x = canvas.input.camera_mouse_down_x
                    + (canvas.input.mouse_down_x - canvas.input.mouse_x);
                canvas.camera.y = canvas.input.camera_mouse_down_y
                    + (canvas.input.mouse_down_y - canvas.input.mouse_y);
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            canvas.input.mouse_down = true;
            canvas.input.mouse_down_x = event->button.x;
            canvas.input.mouse_down_y = event->button.y;
            canvas.input.camera_mouse_down_x = canvas.camera.x;
            canvas.input.camera_mouse_down_y = canvas.camera.y;
        } break;

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            canvas.input.mouse_down = false;
        } break;
    }

    return false;
}

void canvas_draw(void) {
    draw_background();

    {
        // char *str = "THE five boxing WIZARDS jump QUICKLY";
        // draw_text(str, -16, -16, draw_clr_white, 2);
        // draw_rect_outline(draw_text_measure(str, -16, -16, 2), draw_clr_white);

        // draw_text_centered("Xyz", (SDL_Rect) { 100, 100, 100,  50 }, draw_clr_white, 3);
        // draw_text_centered("Xyz", (SDL_Rect) { 100, 100, 100, 100 }, draw_clr_white, 2);
        // draw_text_centered("Xyz", (SDL_Rect) { 100, 150, 100,  50 }, draw_clr_white, 1);
        // draw_rect_outline((SDL_Rect) { 100, 100, 100, 100 }, draw_clr_white);

        // draw_rect((SDL_Rect) { 0, 0, 100, 100 }, in);
        draw_rect_outline((SDL_Rect) { 0, 0, 100, 100 }, draw_clr_white);
        draw_text_centered("xyz", (SDL_Rect) {  0,  0, 100, 80 }, draw_clr_white, 1);
        draw_text_centered("yes", (SDL_Rect) {  0, 80,  50, 20 }, draw_clr_white, 1);
        draw_text_centered( "no", (SDL_Rect) { 50, 80,  50, 20 }, draw_clr_white, 1);

#if 0
        uint32_t in  = draw_rgba(0xff, 0x00, 0xff, 0xff);
        uint32_t out = draw_rgba(0xff, 0xff, 0xff, 0xff);
        draw_rect((SDL_Rect) { 0, 0, 100, 100 }, in);
        draw_rect_outline((SDL_Rect) { 0, 0, 100, 100 }, out);

        draw_rect((SDL_Rect) { 100, 100, 100, 100 }, in);
        draw_rect_outline((SDL_Rect) { 100, 100, 100, 100 }, out);

        draw_rect((SDL_Rect) { 200, 200, 100, 100 }, in);
        draw_rect_outline((SDL_Rect) { 200, 200, 100, 100 }, out);
#endif
    }
}

