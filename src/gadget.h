#ifndef __GADGET_HEADER
#define __GADGET_HEADER

typedef struct {
    uint32_t idx, gen;
} gadget_ID;

typedef enum {
    gadget_Kind_NONE,
    gadget_Kind_Supply,
    gadget_Kind_Component,
    gadget_Kind_TestRig,
    gadget_Kind_Sell,
    /* gadget_Kind_Contract, gadget_Kind_Box, Rent, Debt */
} gadget_Kind;
typedef struct {
    gadget_ID id;
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

void gadget_do(gadget_Gadget *gadget, gadget_Do *doin);

/* allocates space for a new gadget, returns it ZII-ed sans its ID */
gadget_Gadget *gadget_alloc(void);

/* deletes a gadget, freeing its memory for reuse */
void gadget_free(gadget_ID id);

/* returns a pointer to a gadget */
gadget_Gadget *gadget_get(gadget_ID);

#endif
