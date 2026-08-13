#ifndef __GADGET_HEADER
#define __GADGET_HEADER

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

void gadget_do(gadget_Gadget *gadget, gadget_Do *doin);

#endif
