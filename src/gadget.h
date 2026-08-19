#ifndef __GADGET_HEADER
#define __GADGET_HEADER
#include <stdint.h>
#include <SDL3/SDL.h>

typedef uint64_t gadget_ID;
#define gadget_ID_IDX(id) (uint32_t)(id >>  0)
#define gadget_ID_GEN(id) (uint32_t)(id >> 32)
#define gadget_ID_FROM(idx, gen) ((uint64_t)idx | ((uint64_t)gen << 32))
#define gadget_ID_NONE 0 /* (generation 0 is not valid) */

typedef enum {
    gadget_Kind_NONE,
    gadget_Kind_Supply,
    gadget_Kind_Component,
    gadget_Kind_TestRig,
    gadget_Kind_Sell,
    /* gadget_Kind_Contract, gadget_Kind_Box, Rent, Debt */
} gadget_Kind;

typedef enum {
    gadget_ComponentKind_NONE,
    gadget_ComponentKind_X,
    gadget_ComponentKind_Joint,
    gadget_ComponentKind_Aggregate,
} gadget_ComponentKind;

typedef struct {
    gadget_ID id;
    SDL_Rect extents;
    gadget_ID daddy_id, firstborn_id, next_sibling_id;
    char *debug_str;

    gadget_Kind kind;
    union {
        struct {
            gadget_ComponentKind kind;
        } component;
    } kind_data;
} gadget_Gadget;

typedef enum {
    gadget_DoKind_Draw,
    gadget_DoKind_DrawBackground,
    gadget_DoKind_Event,
    // gadget_Event_Tooltip,
} gadget_DoKind;
typedef struct {
    gadget_DoKind kind;
    SDL_Point canvas_mouse;
    union {
        struct {
            bool capture; /* stop canvas from using event */
            SDL_Event *data;
        } event;
    } body;
} gadget_Do;

void gadget_do(gadget_Gadget *gadget, gadget_Do *doin);

/* allocates space for a new gadget, returns it ZII-ed sans its ID */
gadget_ID gadget_alloc(gadget_Gadget init);

/* deletes a gadget, freeing its memory for reuse */
void gadget_free(gadget_ID id);

/* returns a pointer to a gadget */
gadget_Gadget *gadget_get(gadget_ID);

typedef struct {
    gadget_Gadget *g;
    size_t i; /* this is 1 ahead of g's index */
} gadget_Iter;
bool gadget_iter_next(gadget_Iter *iter);

void gadget_init(void);
bool gadget_event(SDL_Event *event, SDL_Point canvas_mouse);

#endif
