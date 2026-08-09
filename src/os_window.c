#include "os_window.h"
#include "draw.h"

static struct {
    /* mouse position/hit test position */
    SDL_Point p;

    /* used to make sure mouse up and mouse down were on same el */
    uint8_t mouse_down_button;

    /* whether or not hit test should return "drag" vs. we are using this mouse input */
    bool drag, capture;

    /* booleans for what mode we're in */
    bool do_draw, do_hit_test, do_capture, do_click, do_mouse_down;

    struct { int mouse_x, mouse_y; } input;
    int size_x, size_y;
} os_window;

/* returns true on click */
static bool os_window_button(os_window_Button button, int x, int y, bool enabled) {
    char *label = "?";
    switch (button) {
        case os_window_Button_ZoomIn:  label = "+"; break;
        case os_window_Button_ZoomOut: label = "-"; break;
        case os_window_Button_Quit:    label = "x"; break;
        case os_window_Button_NONE:    label = "?"; break;
    }

    uint32_t color = draw_clr_text;
    if (!enabled) color = draw_clr_black;

    float cx = x + 8; /* assuming one character label and roughly 8x8 font at x2 scale */
    float cy = y + 8;
    float dst = sqrtf(
        (cx - os_window.p.x)*(cx - os_window.p.x) +
        (cy - os_window.p.y)*(cy - os_window.p.y)
    );
    bool click = false;

    if (enabled && dst < 8) {
        os_window.drag = false;
        color = draw_clr_hilite;

        if (os_window.do_mouse_down)
            os_window.mouse_down_button = (uint8_t) button;

        click = (os_window.mouse_down_button == button) && os_window.do_click;
        if (click) os_window.mouse_down_button = os_window_Button_NONE;
    }
    if (os_window.do_draw) draw_text(label, x, y, color, 2); x += 20;

    return click;
}

bool os_window_fn(os_window_Mode mode, const SDL_Point *p) {

    /* fill in mouse if P is not supplied */
    if (p == NULL) {
        os_window.p.x = os_window.input.mouse_x;
        os_window.p.y = os_window.input.mouse_y;
        p = &os_window.p;
    } else {
        os_window.p = *p;
    }

    bool do_draw = os_window.do_draw = mode == os_window_Mode_Draw;
    os_window.do_hit_test   = mode == os_window_Mode_HitTest;
    os_window.do_capture    = mode == os_window_Mode_Capture;
    os_window.do_click      = mode == os_window_Mode_Click;
    os_window.do_mouse_down = mode == os_window_Mode_MouseDown;

    /* assume drag/capture if mouse over bar */
    {
        bool over_bar = p->y < os_window_TOP_BAR_THICKNESS;
        /* let active drag on the canvas take priority */
        if (canvas.input.mouse_down) over_bar = false;
        os_window.drag = os_window.capture = over_bar;
    }

    /* top bar background */
    if (do_draw) {
        /* shadow */
        draw_rect(
            (SDL_Rect) { 0, 2, os_window.size_x, os_window_TOP_BAR_THICKNESS },
            draw_clr_shadow
        );

        /* background */
        draw_rect(
            (SDL_Rect) { 0, 0, os_window.size_x, os_window_TOP_BAR_THICKNESS },
            draw_clr_bg
        );
    }

    /* content of top bar */
    {
        int x = 5, y = 6;

        if (do_draw) draw_text("SpAT", x, y, draw_clr_text, 2); x += 60;

        x -= 20;
        if (os_window_button(os_window_Button_ZoomIn, x += 20, y, canvas.camera.scale < 5))
            canvas.camera.scale += 1;
        if (os_window_button(os_window_Button_ZoomOut, x += 20, y, canvas.camera.scale > 1))
            canvas.camera.scale -= 1;

        if (os_window_button(os_window_Button_Quit, os_window.size_x - 20, 4, true))
            SDL_Quit();
    }

    /* outline around entire window */
    if (do_draw) draw_rect_outline(
        (SDL_Rect) { 0, 0, os_window.size_x, os_window.size_y },
        draw_clr_bg
    );

    if (os_window.do_hit_test)
        return os_window.drag;
    if (os_window.do_capture)
        return os_window.capture;
    return false;
}

SDL_HitTestResult os_window_hit_test(SDL_Window *win, const SDL_Point *p, void *data) {
    if (os_window_fn(os_window_Mode_HitTest, p)) {
        /* if the hit test is failing you should NOT be trusting your mouse input */
        os_window.input.mouse_x = 0;
        os_window.input.mouse_y = 0;
        os_window.mouse_down_button = os_window_Button_NONE;
        return SDL_HITTEST_DRAGGABLE;
    }

    return SDL_HITTEST_NORMAL;
}

bool os_window_event(SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: {
            return SDL_APP_SUCCESS;
        } break;

        case SDL_EVENT_MOUSE_MOTION: {
            os_window.input.mouse_x = event->motion.x;
            os_window.input.mouse_y = event->motion.y;

            SDL_Point p = { event->motion.x, event->motion.y };
            if (os_window_fn(os_window_Mode_Capture, &p))
                return true;
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            SDL_Point p = { event->button.x, event->button.y };
            if (os_window_fn(os_window_Mode_Capture, &p)) {
                os_window_fn(os_window_Mode_MouseDown, &p);
                return true;
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            SDL_Point p = { event->button.x, event->button.y };

            os_window_fn(os_window_Mode_Click, &p);
        } break;
    }

    return false;
}

void os_window_set_size(int x, int y) {
    os_window.size_x = x;
    os_window.size_y = y;
}
