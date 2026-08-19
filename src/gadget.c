#include "gadget.h"
#include "rect.h"
#include "draw.h"
#include "app.h"

/* actions that change The Shape of The Tree are queued
 * up and played out after the Eventing is over, to prevent
 * weird situations where gadgets don't get the updates they need. */
typedef enum {
    gadget_ActionKind_NONE,
    gadget_ActionKind_ChildTake,
    gadget_ActionKind_ChildGive,
    gadget_ActionKind_HeldSet,
} gadget_ActionKind;
typedef struct {
    gadget_ActionKind kind;
    union {
        struct { gadget_ID child, daddy; } child_take;
        struct { gadget_ID child, daddy; } child_give;
        struct { gadget_ID held; } held_set;
    } kind_data;
} gadget_Action;

typedef struct {
    /* TODO: intrusive freelist/active skiplist */
    uint32_t gen;
    gadget_Gadget gadget;
} gadget_Entry;

static struct {
    gadget_Entry *entries;
    uint32_t entries_capacity;

    gadget_Action actions[10];
    int actions_next_idx;

    /* don't set held_gadget directly as some gadgets like to see
     * who used to be held on SDL_EVENT_MOUSE_BUTTON_UP and won't see
     * that you were the held one if your Event handler runs before theirs.
     *
     * instead use gadget_held_set  */
    gadget_ID held_gadget;
} state = {0};

static void gadget_action_queue(gadget_Action a) {
    assert(state.actions_next_idx < countof(state.actions));
    state.actions[state.actions_next_idx++] = a;
}

/* queues adding a child to a gadget */
static void gadget_held_set(gadget_ID held) {
    gadget_action_queue((gadget_Action) {
        .kind = gadget_ActionKind_HeldSet,
        .kind_data.held_set = { .held = held },
    });
}

/* queues adding a child to a gadget */
static void gadget_child_give(gadget_ID daddy, gadget_ID child) {
    assert(child && daddy);
    log_trace("queueing giving '%s' to '%s',\n", gadget_get(child)->debug_str,
                                                 gadget_get(daddy)->debug_str);
    gadget_action_queue((gadget_Action) {
        .kind = gadget_ActionKind_ChildGive,
        .kind_data.child_give = { .daddy = daddy, .child = child },
    });
}

/* queues removing a child from a gadget */
static void gadget_child_take(gadget_ID daddy, gadget_ID child) {
    assert(child && daddy);
    log_trace("queueing taking '%s' from '%s',\n", gadget_get(child)->debug_str,
                                                   gadget_get(daddy)->debug_str);
    gadget_action_queue((gadget_Action) {
        .kind = gadget_ActionKind_ChildTake,
        .kind_data.child_take = { .daddy = daddy, .child = child },
    });
}

static void gadget_actions_execute(void) {
    for (int i = 0; i < state.actions_next_idx; i++) {
        gadget_Action action = state.actions[i];

        switch (action.kind) {

            case gadget_ActionKind_NONE: break;

            case gadget_ActionKind_HeldSet: state.held_gadget = action.kind_data.held_set.held; break;

            case gadget_ActionKind_ChildGive: {

                gadget_ID child = action.kind_data.child_give.child;
                gadget_ID daddy = action.kind_data.child_give.daddy;

                log_trace("giving '%s' to '%s',\n", gadget_get(child)->debug_str,
                                                    gadget_get(daddy)->debug_str);

                gadget_get(child)->daddy_id = daddy;

                gadget_ID sibling = gadget_get(daddy)->firstborn_id;
                if (sibling == gadget_ID_NONE) {
                     gadget_get(daddy)->firstborn_id = child;
                     continue; // success = true;
                }

                for (;;) {
                    if (sibling == child) break; // success = false;
                    gadget_ID next_sibling = gadget_get(sibling)->next_sibling_id;
                    if (!next_sibling) {
                        gadget_get(sibling)->next_sibling_id = child;
                        break; // success = true;
                    }
                    sibling = next_sibling;
                }
            } break;

            case gadget_ActionKind_ChildTake: {
                gadget_ID child = action.kind_data.child_take.child;
                gadget_ID daddy = action.kind_data.child_take.daddy;

                log_trace("taking '%s' from '%s',\n", gadget_get(child)->debug_str,
                                                      gadget_get(daddy)->debug_str);

                gadget_get(child)->daddy_id = gadget_ID_NONE;

                gadget_ID sibling = gadget_get(daddy)->firstborn_id;
                if (sibling == child) {
                    gadget_get(daddy)->firstborn_id = gadget_get(sibling)->next_sibling_id;
                    continue; // success = true;
                }

                for (;;) {
                    if (sibling == gadget_ID_NONE) break; // success = false;
                    if (gadget_get(sibling)->next_sibling_id == child) {
                        gadget_get(sibling)->next_sibling_id = gadget_get(child)->next_sibling_id;
                        break; // success = true;
                    }
                    sibling = gadget_get(sibling)->next_sibling_id;
                }
                gadget_get(child)->next_sibling_id = gadget_ID_NONE;
            } break;
        }
    }

    // SDL_zerop(state.actions);
    memset(state.actions, 0, sizeof(state.actions));
    state.actions_next_idx = 0;
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

static bool gadget_sell_clamp_to_grid(
    gadget_Gadget *sell,
    SDL_Rect *to_clamp,
    gadget_ID gadget_to_clamp,
    SDL_Rect grid_area
) {
    int GRID_SIZE = 10;

    assert(to_clamp);

    SDL_Rect clamped = *to_clamp;
    {
        clamped.x -= grid_area.x;
        clamped.y -= grid_area.y;

        clamped.x = clamped.x / GRID_SIZE * GRID_SIZE;
        clamped.y = clamped.y / GRID_SIZE * GRID_SIZE;

        clamped.x += grid_area.x;
        clamped.y += grid_area.y;

        clamped.w = ((clamped.w + (GRID_SIZE-1)) / GRID_SIZE) * GRID_SIZE;
        clamped.h = ((clamped.h + (GRID_SIZE-1)) / GRID_SIZE) * GRID_SIZE;
    }

    {
        if (clamped.x < grid_area.x) return false;
        if (clamped.y < grid_area.y) return false;
        if ((clamped.x + clamped.w) > (grid_area.x + grid_area.w)) return false;
        if ((clamped.y + clamped.h) > (grid_area.y + grid_area.h)) return false;
    }

    for (gadget_ID child = sell->firstborn_id;
        child;
        child = gadget_get(child)->next_sibling_id
    ) {
        if (child == gadget_to_clamp)
            continue;

        /* held gadget can't push you out of your spot */
        if (child == state.held_gadget)
            continue;

        if (SDL_HasRectIntersection(&gadget_get(child)->extents, &clamped))
            return false;
    }

    *to_clamp = clamped;
    return true;
}

bool gadget_event(SDL_Event *event, SDL_Point canvas_mouse) {
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

    gadget_actions_execute();

    return capture;
}

static bool gadget_do_offer_draggable_child(gadget_Gadget *gadget, gadget_Do *doin, gadget_ID child_id) {
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

static bool gadget_do_take_dropped_child(gadget_Gadget *gadget, gadget_Do *doin) {
    bool mouse_up = (doin->kind == gadget_DoKind_Event) &&
                    (doin->body.event.data->type == SDL_EVENT_MOUSE_BUTTON_UP);
    bool held_hover = state.held_gadget &&
        SDL_HasRectIntersection(&gadget_get(state.held_gadget)->extents, &gadget->extents);

    if (mouse_up && held_hover)
        gadget_child_give(gadget->id, state.held_gadget);

    return held_hover;
}

static bool gadget_do_take_one_dropped_child(gadget_Gadget *gadget, gadget_Do *doin) {
    /* need to either be childless, or already holding this child */
    if (gadget_childless(gadget->id) || gadget_child_has(gadget->id, state.held_gadget))
        return gadget_do_take_dropped_child(gadget, doin);
    return false;
}


void gadget_do(gadget_Gadget *gadget, gadget_Do *doin) {
    bool do_draw    = doin->kind == gadget_DoKind_Draw;
    bool do_draw_bg = doin->kind == gadget_DoKind_DrawBackground;
    bool do_event   = doin->kind == gadget_DoKind_Event;

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

            /* child management - supply */
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

            /* child management - test rig */
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
            bool block_drag_start = false;

            switch (gadget->kind_data.component.kind) {
                case gadget_ComponentKind_Aggregate:
                case gadget_ComponentKind_NONE: {
                    if (do_draw) draw_text_centered("?"  , area, draw_clr_white, 2);
                } break;

                case gadget_ComponentKind_Joint: {
                    // if (do_draw) draw_text_centered("|", area, draw_clr_white, 2);

                    int child_count = 0;
                    for (gadget_ID child = gadget->firstborn_id;
                        child;
                        child = gadget_get(child)->next_sibling_id
                    )
                        child_count += 1;

                    SDL_Rect extent = gadget->extents;
                    SDL_Rect last_rect = {0};
                    int og_w = extent.w;
                    for (gadget_ID child = gadget->firstborn_id;
                        child;
                        child = gadget_get(child)->next_sibling_id
                    ) {
                        SDL_Rect child_extents = rect_cut_left(&extent, og_w / child_count);

                        gadget_get(child)->extents = child_extents;
                        gadget_do(gadget_get(child), doin);

                        if (last_rect.w) {
                            SDL_Rect space = rect_union(
                                rect_cut_right(&last_rect    , 2),
                                rect_cut_left (&child_extents, 2)
                            );
                            if (do_draw) draw_text_centered("|", space, draw_clr_white, 2);
                            if (SDL_PointInRect(&canvas_mouse, &space)) {
                                if (do_draw) draw_rect_outline(space, draw_clr_hilite);
                                block_drag_start = true;
                            }
                        }
                        last_rect = child_extents;
                    }

                } break;

                case gadget_ComponentKind_X: {
                    if (do_draw) draw_text_centered("x"  , area, draw_clr_white, 2);
                } break;
            }

            if ((held || hover) && !block_drag_start) {
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
                            gadget_held_set(gadget->id);
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
                            gadget_held_set(gadget_ID_NONE);
                            mouse_down = false;
                        } break;
                    }
                }
            }
        } break;

        case gadget_Kind_Sell: {
            uint32_t grid_empty_lite = do_draw_bg ? draw_rgba(0x22, 0x28, 0x22, 0xFF) : 0;
            uint32_t grid_empty_dark = do_draw_bg ? draw_rgba(0x11, 0x18, 0x11, 0xFF) : 0;
            uint32_t grid_full_lite  = do_draw_bg ? draw_rgba(0x12, 0x18, 0x12, 0xFF) : 0;
            uint32_t grid_full_dark  = do_draw_bg ? draw_rgba(0x08, 0x10, 0x08, 0xFF) : 0;
            uint32_t sell_area_outline = draw_rgba(0xAA, 0xFF, 0xAA, 0xFF);
            uint32_t sell_item_outline = draw_rgba(0x99, 0xBB, 0x99, 0xFF);

            SDL_Rect sell_area = gadget->extents;
            if (do_draw) draw_rect_outline(sell_area, sell_area_outline);

            if (do_draw_bg) draw_checkerboard(sell_area, 20, grid_empty_lite, grid_empty_dark);

            SDL_Rect title = rect_cut_top(&sell_area, 10);
            if (do_draw) draw_text_centered("SELL", title, draw_clr_title, 1);

            /* child management - sell */
            {

                for (gadget_ID child = gadget->firstborn_id;
                    child;
                    child = gadget_get(child)->next_sibling_id
                ) {
                    SDL_Rect clamped = gadget_get(child)->extents;
                    bool has_space = gadget_sell_clamp_to_grid(gadget, &clamped, child, sell_area);

                    if (has_space) {
                        if (do_draw_bg) draw_checkerboard(clamped, 20, grid_full_lite, grid_full_dark);
                        if (do_draw) draw_rect_outline(clamped, sell_item_outline);
                    }
                    gadget_do(gadget_get(child), doin);

                    if (gadget_do_offer_draggable_child(gadget, doin, child)) {
                        if (has_space)
                            gadget_get(child)->extents =
                                rect_centered_in(clamped, gadget_get(child)->extents);
                    }
                }

                if (state.held_gadget) {
                    SDL_Rect extents = gadget_get(state.held_gadget)->extents;
                    if (gadget_sell_clamp_to_grid(gadget, &extents, state.held_gadget, sell_area)) {
                        if (gadget_do_take_dropped_child(gadget, doin) && do_draw)
                            draw_rect_outline(extents, draw_clr_hilite);
                    }
                }

                /* important to release currently held one
                 * before updating others, or it will kick them out */
                if (state.held_gadget && gadget_child_has(gadget->id, state.held_gadget)) {
                    bool mouse_up = (doin->kind == gadget_DoKind_Event) &&
                                    (doin->body.event.data->type == SDL_EVENT_MOUSE_BUTTON_UP);
                    gadget_ID held = state.held_gadget;
                    SDL_Rect clamped = gadget_get(held)->extents;
                    bool has_space = gadget_sell_clamp_to_grid(gadget, &clamped, held, sell_area);

                    if (mouse_up && !has_space) {
                        gadget_child_take(gadget->id, held);
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
            .debug_str = "supply",
            .extents = { 0, 0, 100, 100 },
            .kind = gadget_Kind_Supply
        });

        gadget_ID joint = gadget_alloc((gadget_Gadget) {
            .extents = rect_inflate(draw_text_measure("xxx", 150,  50, 2), 5),
            .kind = gadget_Kind_Component,
            .kind_data.component.kind = gadget_ComponentKind_Joint,
            .debug_str = "supply xyz",
        });
        gadget_child_give(supply, joint);

        gadget_ID x0 = gadget_alloc((gadget_Gadget) {
            .kind = gadget_Kind_Component,
            .kind_data.component.kind = gadget_ComponentKind_X,
        });
        gadget_child_give(joint, x0);

        gadget_ID x1 = gadget_alloc((gadget_Gadget) {
            .kind = gadget_Kind_Component,
            .kind_data.component.kind = gadget_ComponentKind_X,
        });
        gadget_child_give(joint, x1);

        gadget_ID x2 = gadget_alloc((gadget_Gadget) {
            .kind = gadget_Kind_Component,
            .kind_data.component.kind = gadget_ComponentKind_X,
        });
        gadget_child_give(joint, x2);
    }

    gadget_alloc((gadget_Gadget) {
        .extents = { 150, 100,  50,  50 },
        .kind = gadget_Kind_TestRig,
        .debug_str = "test rig",
    });
    gadget_alloc((gadget_Gadget) {
        .extents = { 250,   0, 100, 100 },
        .kind = gadget_Kind_Sell,
        .debug_str = "sell",
    });

    gadget_alloc((gadget_Gadget) {
        .extents = rect_inflate(draw_text_measure("x", 120,  80, 2), 2),
        .kind = gadget_Kind_Component,
        .kind_data.component.kind = gadget_ComponentKind_X,
        .debug_str = "x",
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
