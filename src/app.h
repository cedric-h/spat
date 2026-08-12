#ifndef __APP_HEADER
#define __APP_HEADER

#include <SDL3/SDL.h>

#define dbg() __builtin_debugtrap()
#define spat_min(x, y) (((x) < (y)) ? (x) : (y))
#define spat_max(x, y) (((x) > (y)) ? (x) : (y))
#define countof(arr) (int)( sizeof(arr) / sizeof((arr)[0]) )

typedef enum {
    app_Cursor_Default,
    app_Cursor_Pointer,
    app_Cursor_Move,
    app_Cursor_Grab,
    app_Cursor_Grabbing,
    // NO_DROP
    app_Cursor_COUNT,
} app_Cursor;

typedef struct {
    SDL_Rect canvas_window_rect;
    SDL_Window *window;
    app_Cursor cursor;
} App;

void app_cursors_init(void);
void app_cursors_update(void);
void app_cursors_free(void);

App app;

#endif
