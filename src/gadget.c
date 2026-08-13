#include "gadget.h"

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
