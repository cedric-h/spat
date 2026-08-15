#include "canvas.h"
#include "draw.h"
#include "rect.h"
#include "gadget.h"
#include "app.h"

canvas_Canvas canvas = {
    .camera.scale = 2,
    .camera.x = -50,
    .camera.y = -50,
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

    if (gadget_event(event, canvas.mouse))
        return true;

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
            app.drag_owner = app_DragOwner_Canvas;
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
    draw_grid(100);
    app.cursor = app_Cursor_Move;

    gadget_Do doin = { .canvas_mouse = canvas.mouse };

    doin.kind = gadget_DoKind_DrawBackground;
    for (gadget_Iter i = {0}; gadget_iter_next(&i);) gadget_do(i.g, &doin);

    doin.kind = gadget_DoKind_Draw;
    for (gadget_Iter i = {0}; gadget_iter_next(&i);) gadget_do(i.g, &doin);
}

/* disgusting, but used by os_window to not interrupt canvas actions for dragging window */
bool canvas_input_is_mouse_down() {
    return canvas.input.is_mouse_down;
}
