#include "canvas.h"
#include "draw.h"
#include "rect.h"

typedef enum {
    gadget_Kind_NONE,
    gadget_Kind_Supply,
    gadget_Kind_Component,
    gadget_Kind_TestRig,
    gadget_Kind_Sell,
    /* gadget_Kind_Contract, gadget_Kind_Box, Rent, Debt */
} gadget_Kind;
typedef struct {
    gadget_Kind kind;
    SDL_Rect extents;
} gadget_Gadget;

typedef enum {
    gadget_DoKind_Draw,
    gadget_DoKind_Event,
    // gadget_Event_Tooltip,
} gadget_DoKind;
typedef struct {
    gadget_DoKind kind;
    union {
        struct {
            bool capture; /* stop canvas from using event */
            SDL_Event *data;
        } event;
    } body;
} gadget_Do;

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

void gadget_do(gadget_Gadget *gadget, gadget_Do *doin) {
    bool do_draw  = doin->kind == gadget_DoKind_Draw;
    bool do_event = doin->kind == gadget_DoKind_Event;

    switch (gadget->kind) {

        case gadget_Kind_NONE: {
        } break;

        case gadget_Kind_Supply: {
            SDL_Rect supply_rect = gadget->extents;

            if (do_draw) draw_rect_outline(supply_rect, draw_rgba(0xFF, 0xFF, 0xAA, 0xFF));

            SDL_Rect title = rect_cut_top(&supply_rect, 15);
            if (do_draw) draw_text_centered("SUPPLY", title, draw_clr_title, 1);

            SDL_Rect options = rect_cut_bottom(&supply_rect, 30);
            if (do_draw) draw_text_centered("xyz", supply_rect, draw_clr_white, 2);

            SDL_Rect yes = options;
            SDL_Rect  no = rect_cut_right(&yes, 50);
            rect_shrink(&yes, 7);
            rect_shrink(& no, 7);

            if (do_draw) draw_text_centered("yes", yes, draw_clr_white, 1);
            if (do_draw && SDL_PointInRect(&canvas.mouse, &yes))
                draw_rect_outline(yes, draw_clr_white), app.cursor = app_Cursor_Pointer;

            if (do_draw) draw_text_centered( "no",  no, draw_clr_white, 1);
            if (do_draw && SDL_PointInRect(&canvas.mouse, &no))
                draw_rect_outline(no, draw_clr_white), app.cursor = app_Cursor_Pointer;
        } break;

        case gadget_Kind_TestRig: {
            SDL_Rect test_area = gadget->extents;
            if (do_draw) draw_rect_outline(test_area, draw_rgba(0xAA, 0xAA, 0xFF, 0xFF));

            SDL_Rect title = rect_cut_top(&test_area, 15);
            if (do_draw) draw_text_centered("TEST", title, draw_clr_title, 1);
        } break;

        case gadget_Kind_Component: {
            SDL_Rect area = gadget->extents;
            if (do_draw) draw_text_centered("xyz", area, draw_clr_white, 2);

            if (SDL_PointInRect(&canvas.mouse, &area)) {
                if (do_draw) app.cursor = app_Cursor_Grab;
                if (do_event) {
                    doin->body.event.capture = true;

                    static SDL_Point comp_mouse_down_pos, canvas_mouse_down_pos;
                    static bool mouse_down;
                    SDL_Event *event = doin->body.event.data;
                    switch (event->type) {
                        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                            canvas_mouse_down_pos = canvas.mouse;
                            comp_mouse_down_pos = (SDL_Point) { gadget->extents.x, gadget->extents.y };
                            mouse_down = true;
                        } break;

                        case SDL_EVENT_MOUSE_MOTION: {
                            if (mouse_down) {
                                gadget->extents.x = comp_mouse_down_pos.x + (canvas.mouse.x - canvas_mouse_down_pos.x);
                                gadget->extents.y = comp_mouse_down_pos.y + (canvas.mouse.y - canvas_mouse_down_pos.y);
                            }
                        } break;

                        case SDL_EVENT_MOUSE_BUTTON_UP: {
                            mouse_down = false;
                        } break;
                    }
                }
            }
        } break;

        case gadget_Kind_Sell: {
            SDL_Rect sell_area = gadget->extents;
            if (do_draw) draw_rect_outline(sell_area, draw_rgba(0xAA, 0xFF, 0xAA, 0xFF));

            SDL_Rect title = rect_cut_top(&sell_area, 15);
            if (do_draw) draw_text_centered("SELL", title, draw_clr_title, 1);

        } break;

    }
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
