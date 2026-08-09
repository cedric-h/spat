#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "draw.h"
#include <math.h>

typedef enum {
    os_window_Mode_Draw,
    os_window_Mode_HitTest,
    os_window_Mode_Capture,
    os_window_Mode_Click,
    os_window_Mode_MouseDown,
} os_window_Mode;
static bool os_window_fn(os_window_Mode mode, const SDL_Point *p);

static struct {
    /* mouse position/hit test position */
    const SDL_Point *p;

    /* used to make sure mouse up and mouse down were on same el */
    uint8_t mouse_down_button;

    /* whether or not hit test should return "drag" vs. we are using this mouse input */
    bool drag, capture;

    /* booleans for what mode we're in */
    bool do_draw, do_hit_test, do_capture, do_click, do_mouse_down;
} os_window;

#define clr_bg     draw_rgba(0x33, 0x33, 0x33, 0xFF)
#define clr_shadow draw_rgba(0x08, 0x08, 0x08, 0xFF)
#define clr_text   draw_rgba(0x66, 0x66, 0x66, 0xFF)
#define clr_black  draw_rgba(0x11, 0x11, 0x11, 0xFF)
#define clr_hilite draw_rgba(0xFF, 0xAA, 0xAA, 0xFF)

/* returns true on click */
typedef enum {
   os_window_Button_ZoomIn,
   os_window_Button_ZoomOut,
   os_window_Button_Quit,
   os_window_Button_NONE,
} os_window_Button;
static bool os_window_button(os_window_Button button, int x, int y, bool enabled) {
    char *label = "?";
    switch (button) {
        case os_window_Button_ZoomIn:  label = "+"; break;
        case os_window_Button_ZoomOut: label = "-"; break;
        case os_window_Button_Quit:    label = "x"; break;
        case os_window_Button_NONE:    label = "?"; break;
    }

    uint32_t color = clr_text;
    if (!enabled) color = clr_black;

    float cx = x + 8; /* assuming one character label and roughly 8x8 font at x2 scale */
    float cy = y + 8;
    float dst = sqrtf(
        (cx - os_window.p->x)*(cx - os_window.p->x) +
        (cy - os_window.p->y)*(cy - os_window.p->y)
    );
    bool click = false;

    if (enabled && dst < 8) {
        os_window.drag = false;
        color = clr_hilite;

        if (os_window.do_mouse_down)
            os_window.mouse_down_button = (uint8_t) button;

        click = (os_window.mouse_down_button == button) && os_window.do_click;
        if (click) os_window.mouse_down_button = os_window_Button_NONE;
    }
    if (os_window.do_draw) draw_text(label, x, y, color, 2); x += 20;

    return click;
}

static bool os_window_fn(os_window_Mode mode, const SDL_Point *p) {

    os_window.p = p;
    bool do_draw = os_window.do_draw = mode == os_window_Mode_Draw;
    os_window.do_hit_test   = mode == os_window_Mode_HitTest;
    os_window.do_capture    = mode == os_window_Mode_Capture;
    os_window.do_click      = mode == os_window_Mode_Click;
    os_window.do_mouse_down = mode == os_window_Mode_MouseDown;

    /* assume drag/capture if mouse over bar */
    os_window.drag = os_window.capture = p->y < DRAW_OS_WINDOW_THICKNESS;

    /* top bar background */
    if (do_draw) {
        /* shadow */
        draw_rect(
            (SDL_Rect) { 0, 2, app.input.size_x, DRAW_OS_WINDOW_THICKNESS },
            clr_shadow
        );

        /* background */
        draw_rect(
            (SDL_Rect) { 0, 0, app.input.size_x, DRAW_OS_WINDOW_THICKNESS },
            clr_bg
        );
    }

    /* content of top bar */
    {
        int x = 5, y = 6;

        if (do_draw) draw_text("SpAT", x, y, clr_text, 2); x += 60;

        x -= 20;
        if (os_window_button(os_window_Button_ZoomIn, x += 20, y, app.camera.scale < 5))
            app.camera.scale += 1;
        if (os_window_button(os_window_Button_ZoomOut, x += 20, y, app.camera.scale > 1))
            app.camera.scale -= 1;

        if (os_window_button(os_window_Button_Quit, app.input.size_x - 20, 4, true))
            SDL_Quit();
    }

    /* outline around entire window */
    if (do_draw) draw_rect_outline(
        (SDL_Rect) { 0, 0, app.input.size_x, app.input.size_y },
        clr_bg
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
        app.input.window_mouse_x = 0;
        app.input.window_mouse_y = 0;
        os_window.mouse_down_button = os_window_Button_NONE;
        return SDL_HITTEST_DRAGGABLE;
    }

    return SDL_HITTEST_NORMAL;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Space Port Arms Technician", "1.0", "com.problemchild.spat");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    {
        app.input.size_x = 50*16;
        app.input.size_y = 50*9;
        app.window = SDL_CreateWindow(
            "spat",
            app.input.size_x,
            app.input.size_y,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT
        );
        if (app.window == NULL) {
            SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        if (SDL_SetWindowSurfaceVSync(app.window, true))
            SDL_Log("vsync enabled");
        else
            SDL_Log("vsync disabled: %s", SDL_GetError());
    }

    if (!SDL_SetWindowHitTest(app.window, os_window_hit_test, NULL)) {
        SDL_Log("Couldn't set hit test: %s", SDL_GetError());
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: {
            return SDL_APP_SUCCESS;
        } break;

        case SDL_EVENT_MOUSE_MOTION: {
            app.input.window_mouse_x = event->motion.x;
            app.input.window_mouse_y = event->motion.y;

            SDL_Point p = { event->motion.x, event->motion.y };
            if (!os_window_fn(os_window_Mode_Capture, &p)) {
                app.input.canvas_mouse_x = event->motion.x;
                app.input.canvas_mouse_y = event->motion.y;
            }

            if (app.input.canvas_mouse_down) {
                app.camera.x = app.input.camera_mouse_down_x
                    + (app.input.canvas_mouse_down_x - app.input.canvas_mouse_x);
                app.camera.y = app.input.camera_mouse_down_y
                    + (app.input.canvas_mouse_down_y - app.input.canvas_mouse_y);
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            SDL_Point p = { event->button.x, event->button.y };
            if (os_window_fn(os_window_Mode_Capture, &p))
                os_window_fn(os_window_Mode_MouseDown, &p);
            else {
                app.input.canvas_mouse_down = true;
                app.input.canvas_mouse_down_x = event->button.x;
                app.input.canvas_mouse_down_y = event->button.y;
                app.input.camera_mouse_down_x = app.camera.x;
                app.input.camera_mouse_down_y = app.camera.y;
            }
        } break;

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            SDL_Point p = { event->button.x, event->button.y };

            os_window_fn(os_window_Mode_Click, &p);

            app.input.canvas_mouse_down = false;
        } break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    SDL_Surface *surface = SDL_GetWindowSurface(app.window);
    if (surface == NULL) {
        SDL_Log("Couldn't get framebuffer surface: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    app.input.size_x = surface->w;
    app.input.size_y = surface->h;

    {
        SDL_Surface *canvas = SDL_CreateSurfaceFrom(
            surface->w,
            surface->h - DRAW_OS_WINDOW_THICKNESS,
            surface->format,
            surface->pixels + surface->pitch * DRAW_OS_WINDOW_THICKNESS,
            surface->pitch
        );
        draw_to(canvas, app.camera);
        draw_background();

        {
            draw_text(
                "The five boxing wizards jump quickly",
                5,
                5,
                SDL_MapSurfaceRGBA(draw.surface, 0xff, 0xff, 0xff, 0xff),
                1
            );

            uint32_t in  = SDL_MapSurfaceRGBA(draw.surface, 0xff, 0x00, 0xff, 0xff);
            uint32_t out = SDL_MapSurfaceRGBA(draw.surface, 0xff, 0xff, 0xff, 0xff);
            draw_rect((SDL_Rect) { 0, 0, 100, 100 }, in);
            draw_rect_outline((SDL_Rect) { 0, 0, 100, 100 }, out);

            draw_rect((SDL_Rect) { 100, 100, 100, 100 }, in);
            draw_rect_outline((SDL_Rect) { 100, 100, 100, 100 }, out);

            draw_rect((SDL_Rect) { 200, 200, 100, 100 }, in);
            draw_rect_outline((SDL_Rect) { 200, 200, 100, 100 }, out);
        }

        SDL_DestroySurface(canvas);
    }

    draw.surface = surface;
    draw.camera = (draw_Camera) { .scale = 1 };

    SDL_Point p = { app.input.window_mouse_x, app.input.window_mouse_y };
    os_window_fn(os_window_Mode_Draw, &p);

    SDL_UpdateWindowSurface(app.window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}
