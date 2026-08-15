#include "gadget.h"
#include "rect.h"
#include "draw.h"
#include "app.h"

static bool gadget_child_has(gadget_ID daddy, gadget_ID child);

/* adds a child to a gadget - returns true if the child was not already present */
static bool gadget_child_add(gadget_ID daddy, gadget_ID child) {
    gadget_get(child)->daddy_id = daddy;

    gadget_ID sibling = gadget_get(daddy)->firstborn_id;
    if (sibling == gadget_ID_NONE) {
         gadget_get(daddy)->firstborn_id = child;
         return true;
    }

    for (;;) {
        if (sibling == child) return false;
        gadget_ID next_sibling = gadget_get(sibling)->next_sibling_id;
        if (!next_sibling) {
            gadget_get(sibling)->next_sibling_id = child;
            return true;
        }
        sibling = next_sibling;
    }
    
    assert(false);
}

/* removes a child from a gadget - returns true if the child existed */
static bool gadget_child_take(gadget_ID daddy, gadget_ID child) {
    gadget_get(child)->daddy_id = gadget_ID_NONE;

    gadget_ID sibling = gadget_get(daddy)->firstborn_id;
    if (sibling == child) {
        gadget_get(daddy)->firstborn_id = gadget_get(sibling)->next_sibling_id;
        return true;
    }

    for (;;) {
        if (sibling == gadget_ID_NONE) return false;
        if (sibling == child) {
            return true;
        }
        sibling = gadget_get(sibling)->next_sibling_id;
    }
}

/* returns true if a gadget has no children */
static bool gadget_childless(gadget_ID gadget) {
    return gadget_get(gadget)->firstborn_id == gadget_ID_NONE;
}

/* returns true if a gadget owns this child */
static bool gadget_child_has(gadget_ID daddy, gadget_ID child) {
    gadget_ID sibling = gadget_get(daddy)->firstborn_id;

    for (;;) {
        if (sibling == gadget_ID_NONE) return false;
        if (sibling == child) return true;
        sibling = gadget_get(sibling)->next_sibling_id;
    }
}

static bool gadget_clamp_to_grid(SDL_Rect *to_clamp, SDL_Rect grid_area) {
    int GRID_SIZE = 10;

    assert(to_clamp);

    SDL_Rect clamped = *to_clamp;
    clamped.x -= grid_area.x;
    clamped.y -= grid_area.y;

    clamped.x = clamped.x / GRID_SIZE * GRID_SIZE;
    clamped.y = clamped.y / GRID_SIZE * GRID_SIZE;

    // SDL_Point p = rect_clamp_point(grid_area, (SDL_Point) { clamped.x, clamped.y });
    clamped.x += grid_area.x;
    clamped.y += grid_area.y;

    clamped.w = ((clamped.w + (GRID_SIZE-1)) / GRID_SIZE) * GRID_SIZE;
    clamped.h = ((clamped.h + (GRID_SIZE-1)) / GRID_SIZE) * GRID_SIZE;

    if (clamped.x < grid_area.x) return false;
    if (clamped.y < grid_area.y) return false;
    if ((clamped.x + clamped.w) > (grid_area.x + grid_area.w)) return false;
    if ((clamped.y + clamped.h) > (grid_area.y + grid_area.h)) return false;
    *to_clamp = clamped;
    return true;
}

typedef struct {
    /* TODO: intrusive freelist/active skiplist */
    uint32_t gen;
    gadget_Gadget gadget;
} gadget_Entry;

static struct {
    gadget_Entry *entries;
    uint32_t entries_capacity;

    /* don't set held_gadget directly as some gadgets like to see
     * who used to be held on SDL_EVENT_MOUSE_BUTTON_UP and won't see
     * that you were the held one if your Event handler runs before theirs.
     *
     * instead set held_gadget_next */
    gadget_ID held_gadget, held_gadget_next;
} state = {0};

bool gadget_event(SDL_Event *event, SDL_Point canvas_mouse) {
    state.held_gadget_next = state.held_gadget;

    gadget_Do doin = {
        .kind = gadget_DoKind_Event,
        .canvas_mouse = canvas_mouse,
        .body = { .event = { .data = event } },
    };

    bool capture = false;
    for (gadget_Iter i = {0}; gadget_iter_next(&i);) {
        gadget_do(i.g, &doin);

        if (doin.body.event.capture) {
            capture = true;
            break;
        }
    }

    state.held_gadget = state.held_gadget_next;

    return capture;
}

bool gadget_do_offer_draggable_child(gadget_Gadget *gadget, gadget_Do *doin, gadget_ID child_id) {
    bool mouse_up = (doin->kind == gadget_DoKind_Event) &&
                    (doin->body.event.data->type == SDL_EVENT_MOUSE_BUTTON_UP);

    if (!child_id) return false;

    if (state.held_gadget != child_id)
        return true;
    else if (mouse_up) {
        gadget_Gadget *child = gadget_get(child_id);

        /* if the child is released outside our extents, relinquish it */
        if (!SDL_HasRectIntersection(&child->extents, &gadget->extents))
            gadget_child_take(gadget->id, child->id);

    }

    return false;
}

bool gadget_do_take_dropped_child(gadget_Gadget *gadget, gadget_Do *doin) {
    bool mouse_up = (doin->kind == gadget_DoKind_Event) &&
                    (doin->body.event.data->type == SDL_EVENT_MOUSE_BUTTON_UP);
    bool held_hover = state.held_gadget &&
        SDL_HasRectIntersection(&gadget_get(state.held_gadget)->extents, &gadget->extents);

    if (mouse_up && held_hover) {
        gadget_child_add(gadget->id, state.held_gadget);

        /* we don't want the gadget to double-process the event,
         * but if it hasn't unheld itself even though the mouse is up,
         * it needs to process the event, and now that it is owned by
         * us, it wont get any events at all unless we give them to it. */
        if (state.held_gadget_next == state.held_gadget) {
            gadget_do(gadget_get(state.held_gadget), doin);
        }
    }

    return held_hover;
}

bool gadget_do_take_one_dropped_child(gadget_Gadget *gadget, gadget_Do *doin) {
    /* need to either be childless, or already holding this child */
    if (gadget_childless(gadget->id) || gadget_child_has(gadget->id, state.held_gadget))
        return gadget_do_take_dropped_child(gadget, doin);
    return false;
}


void gadget_do(gadget_Gadget *gadget, gadget_Do *doin) {
    bool do_draw  = doin->kind == gadget_DoKind_Draw;
    bool do_event = doin->kind == gadget_DoKind_Event;

    SDL_Point canvas_mouse = doin->canvas_mouse;

    switch (gadget->kind) {

        case gadget_Kind_NONE: {
        } break;

        case gadget_Kind_Supply: {
            SDL_Rect supply_rect = gadget->extents;

            if (do_draw) draw_rect_outline(supply_rect, draw_rgba(0xFF, 0xFF, 0xAA, 0xFF));

            SDL_Rect title = rect_cut_top(&supply_rect, 15);
            if (do_draw) draw_text_centered("SUPPLY", title, draw_clr_title, 1);

            SDL_Rect options = rect_cut_bottom(&supply_rect, 30);

            /* child management */
            {
                if (gadget_get(gadget->firstborn_id))
                    gadget_do(gadget_get(gadget->firstborn_id), doin);

                if (gadget_do_offer_draggable_child(gadget, doin, gadget->firstborn_id))
                    gadget_get(gadget->firstborn_id)->extents = rect_centered_in(
                        supply_rect,
                        gadget_get(gadget->firstborn_id)->extents
                    );

                /* we only take our own child back, no strangers */
                if (state.held_gadget == gadget->firstborn_id)
                    if (gadget_do_take_one_dropped_child(gadget, doin) && do_draw)
                        draw_rect_outline(
                            rect_centered_in(supply_rect, gadget_get(state.held_gadget)->extents),
                            draw_clr_hilite
                        );
            }

            SDL_Rect yes = options;
            SDL_Rect  no = rect_cut_right(&yes, 50);
            yes = rect_shrink(yes, 7);
             no = rect_shrink( no, 7);

            if (do_draw) draw_text_centered("yes", yes, draw_clr_white, 1);
            if (do_draw && SDL_PointInRect(&canvas_mouse, &yes))
                draw_rect_outline(yes, draw_clr_white), app.cursor = app_Cursor_Pointer;

            if (do_draw) draw_text_centered( "no",  no, draw_clr_white, 1);
            if (do_draw && SDL_PointInRect(&canvas_mouse, &no))
                draw_rect_outline(no, draw_clr_white), app.cursor = app_Cursor_Pointer;
        } break;

        case gadget_Kind_TestRig: {
            SDL_Rect test_area = gadget->extents;
            if (do_draw) draw_rect_outline(test_area, draw_rgba(0xAA, 0xAA, 0xFF, 0xFF));

            SDL_Rect title = rect_cut_top(&test_area, 15);
            if (do_draw) draw_text_centered("TEST", title, draw_clr_title, 1);

            /* child management */
            {
                if (gadget_get(gadget->firstborn_id))
                    gadget_do(gadget_get(gadget->firstborn_id), doin);

                if (gadget_do_offer_draggable_child(gadget, doin, gadget->firstborn_id))
                    gadget_get(gadget->firstborn_id)->extents = rect_centered_in(
                        test_area,
                        gadget_get(gadget->firstborn_id)->extents
                    );

                if (gadget_do_take_one_dropped_child(gadget, doin) && do_draw)
                    draw_rect_outline(
                        rect_centered_in(test_area, gadget_get(state.held_gadget)->extents),
                        draw_clr_hilite
                    );
            }
        } break;

        case gadget_Kind_Component: {
            SDL_Rect area = gadget->extents;
            bool held = state.held_gadget == gadget->id;
            bool hover = state.held_gadget == gadget_ID_NONE && SDL_PointInRect(&canvas_mouse, &area);

            if (do_draw) draw_text_centered("xyz", area, draw_clr_white, 2);

            if (held || hover) {
                if (do_draw) {
                    app.cursor = held ? app_Cursor_Grabbing : app_Cursor_Grab;
                    draw_rect_outline(area, held ? draw_clr_white : draw_clr_title);
                }
                if (do_event) {

                    static SDL_Point comp_mouse_down_pos, canvas_mouse_down_pos;
                    static bool mouse_down;

                    SDL_Event *event = doin->body.event.data;
                    switch (event->type) {
                        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                            state.held_gadget_next = gadget->id;
                            canvas_mouse_down_pos = canvas_mouse;
                            comp_mouse_down_pos = (SDL_Point) { gadget->extents.x, gadget->extents.y };
                            mouse_down = true;
                            doin->body.event.capture = true;
                        } break;

                        case SDL_EVENT_MOUSE_MOTION: {
                            if (held && mouse_down) {
                                gadget->extents.x = comp_mouse_down_pos.x + (canvas_mouse.x - canvas_mouse_down_pos.x);
                                gadget->extents.y = comp_mouse_down_pos.y + (canvas_mouse.y - canvas_mouse_down_pos.y);
                            }
                        } break;

                        case SDL_EVENT_MOUSE_BUTTON_UP: {
                            state.held_gadget_next = gadget_ID_NONE;
                            mouse_down = false;
                        } break;
                    }
                }
            }
        } break;

        case gadget_Kind_Sell: {
            SDL_Rect sell_area = gadget->extents;
            if (do_draw) draw_rect_outline(sell_area, draw_rgba(0xAA, 0xFF, 0xAA, 0xFF));

            SDL_Rect title = rect_cut_top(&sell_area, 10);
            if (do_draw) draw_text_centered("SELL", title, draw_clr_title, 1);

            /* child management */
            {
                for (gadget_ID child = gadget->firstborn_id;
                    child;
                    child = gadget_get(child)->next_sibling_id
                ) {
                    gadget_do(gadget_get(child), doin);

                    if (gadget_do_offer_draggable_child(gadget, doin, child)) {
                        SDL_Rect clamped = gadget_get(child)->extents;
                        gadget_clamp_to_grid(&clamped, sell_area);

                        gadget_get(child)->extents = rect_centered_in(clamped, gadget_get(child)->extents);
                    }
                }

                if (state.held_gadget) {
                    SDL_Rect extents = gadget_get(state.held_gadget)->extents;
                    if (gadget_clamp_to_grid(&extents, sell_area)) {
                        if (gadget_do_take_dropped_child(gadget, doin) && do_draw)
                            draw_rect_outline(extents, draw_clr_hilite);
                    }
                }
            }

        } break;

    }
}

/* allocates space for a new gadget, returns it ZII-ed sans its ID */
gadget_ID gadget_alloc(gadget_Gadget init_g) {
    for (uint32_t i = 0; i < state.entries_capacity; i++) {
        gadget_Entry *e = state.entries + i;
        if (gadget_ID_GEN(e->gadget.id) < e->gen) {
            e->gadget = init_g;
            e->gadget.id = gadget_ID_FROM(i, e->gen);
            return e->gadget.id;
        }
    }

    size_t old_capacity = state.entries_capacity;
    state.entries_capacity = spat_max(1024, state.entries_capacity << 1);
    state.entries = SDL_realloc(state.entries, state.entries_capacity * sizeof(gadget_Entry));
    for (uint32_t i = old_capacity; i < state.entries_capacity; i++) {
        state.entries[i].gadget.id = 0;
        state.entries[i].gen = 1;
    }
    return gadget_alloc(init_g);
}

/* returns a pointer to a gadget. don't hold this for longer than
 * necessary, because any call to alloc can invalidate it.  */
gadget_Gadget *gadget_get(gadget_ID id) {
    size_t idx = gadget_ID_IDX(id);
    size_t gen = gadget_ID_GEN(id);
    /* this gadget_ID points to a dead/future man! */
    if (state.entries[idx].gen != gen) return NULL;
    return &state.entries[idx].gadget;
}

void gadget_free(gadget_ID id) {
    size_t idx = gadget_ID_IDX(id);
    size_t gen = gadget_ID_GEN(id);
    state.entries[idx].gen = spat_max(gen + 1, state.entries[idx].gen); /* idempotent */
}

void gadget_init(void) {
    {
        gadget_ID supply = gadget_alloc((gadget_Gadget) {
            .extents = { 0, 0, 100, 100 },
            .kind = gadget_Kind_Supply
        });

        gadget_ID component = gadget_alloc((gadget_Gadget) {
            .extents = rect_inflate(draw_text_measure("xyz", 150,  50, 2), 5),
            .kind = gadget_Kind_Component,
        });
        gadget_child_add(supply, component);
    }

    gadget_alloc((gadget_Gadget) { .extents = { 150, 100,  50,  50 }, .kind = gadget_Kind_TestRig   });
    gadget_alloc((gadget_Gadget) { .extents = { 250,   0, 100, 100 }, .kind = gadget_Kind_Sell      });

    gadget_alloc((gadget_Gadget) {
        .extents = rect_inflate(draw_text_measure("xyz", 150, 100, 2), 5),
        .kind = gadget_Kind_Component,
    });
}

bool gadget_iter_next(gadget_Iter *iter) {
    do {
        iter->g = &(state.entries + iter->i)->gadget;
        iter->i++;

        if (!(iter->i < state.entries_capacity)) break;
        if (iter->g->kind == gadget_Kind_NONE) continue;
        if (iter->g->daddy_id != gadget_ID_NONE) continue;
        break;
    } while (true);

    return iter->i < state.entries_capacity;
}
