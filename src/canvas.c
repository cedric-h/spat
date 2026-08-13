#include "canvas.h"
#include "draw.h"
#include "rect.h"
#include "gadget.h"

static struct {
    struct {
        /* all of these are in screenspace, not worldspace (see canvas.mouse for worldspace) */
        SDL_Point mouse_down, camera_mouse_down, mouse;
        bool is_mouse_down;
    } input;

    /* mouse in world position (mouse position in input is in screen position) */
    SDL_Point mouse;

    draw_Camera camera;

    gadget_Gadget gadgets[100];
} canvas = {
    .camera.scale = 2,
    .camera.x = -50,
    .camera.y = -50,

    .gadgets = {
        (gadget_Gadget) { .extents = {   0,   0, 100, 100 }, .kind = gadget_Kind_Supply    },
        (gadget_Gadget) { .extents = { 150,  50,  30,  30 }, .kind = gadget_Kind_Component },
        (gadget_Gadget) { .extents = { 150, 100,  30,  30 }, .kind = gadget_Kind_Component },
        (gadget_Gadget) { .extents = { 150, 100,  50,  50 }, .kind = gadget_Kind_TestRig   },
        (gadget_Gadget) { .extents = { 250,   0, 100, 100 }, .kind = gadget_Kind_Sell      },
    },
};

static SDL_Point canvas_screen_to_world(SDL_Point screen) {
    float x = screen.x - app.canvas_window_rect.x;
    float y = screen.y - app.canvas_window_rect.y;

    x += canvas.camera.x;
    y += canvas.camera.y;

    x /= canvas.camera.scale;
    y /= canvas.camera.scale;

    return (SDL_Point) { x, y };
}

/* returns true to capture input, though that's not really meaningful here */
bool canvas_event(SDL_Event *event) {

    if (event->type == SDL_EVENT_MOUSE_MOTION)
        canvas.mouse = canvas_screen_to_world(
            (SDL_Point) { event->motion.x, event->motion.y }
        );

    gadget_Do doin = {
        .kind = gadget_DoKind_Event,
        .body = { .event = { .data = event } },
    };
    for (int i = 0; i < countof(canvas.gadgets); i++) {
        gadget_Gadget *g = canvas.gadgets + i;
        if (g->kind == gadget_Kind_NONE) continue;
        gadget_do(g, &doin);

        if (doin.body.event.capture) return true;
    }

    switch (event->type) {
        case SDL_EVENT_QUIT: {
            return SDL_APP_SUCCESS;
        } break;

        case SDL_EVENT_MOUSE_MOTION: {
            canvas.input.mouse.x = event->motion.x;
            canvas.input.mouse.y = event->motion.y;

            if (canvas.input.is_mouse_down) {
                canvas.camera.x = canvas.input.camera_mouse_down.x
                    + (canvas.input.mouse_down.x - canvas.input.mouse.x);
                canvas.camera.y = canvas.input.camera_mouse_down.y
                    + (canvas.input.mouse_down.y - canvas.input.mouse.y);
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            canvas.input.is_mouse_down = true;
            canvas.input.mouse_down.x = event->button.x;
            canvas.input.mouse_down.y = event->button.y;
            canvas.input.camera_mouse_down.x = canvas.camera.x;
            canvas.input.camera_mouse_down.y = canvas.camera.y;
        } break;

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            canvas.input.is_mouse_down = false;
        } break;
    }

    return false;
}

void canvas_draw(void) {
    draw_background();
    app.cursor = app_Cursor_Move;

    for (int i = 0; i < countof(canvas.gadgets); i++) {
        gadget_Gadget *g = canvas.gadgets + i;
        if (g->kind == gadget_Kind_NONE) continue;
        gadget_do(g, &(gadget_Do) { .kind = gadget_DoKind_Draw });
    }
}
