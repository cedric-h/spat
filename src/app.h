#ifndef __APP_HEADER
#define __APP_HEADER

#include <SDL3/SDL.h>
#include <assert.h>

#define dbg() __builtin_debugtrap()
#define spat_min(x, y) (((x) < (y)) ? (x) : (y))
#define spat_max(x, y) (((x) > (y)) ? (x) : (y))
#define spat_clamp(min, max, v) spat_max(min, spat_min(max, v))
#define countof(arr) (int)( sizeof(arr) / sizeof((arr)[0]) )
#define log_trace(...) SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

typedef enum {
    app_Cursor_Default,
    app_Cursor_Pointer,
    app_Cursor_Move,
    app_Cursor_Grab,
    app_Cursor_Grabbing,
    // NO_DROP
    app_Cursor_COUNT,
} app_Cursor;

typedef enum {
    app_DragOwner_NONE,
    app_DragOwner_OsWindow,
    app_DragOwner_Canvas,
} app_DragOwner;

typedef struct {
    SDL_Rect canvas_window_rect;
    SDL_Window *window;
    app_Cursor cursor;
    app_DragOwner drag_owner;
} App;

void app_cursors_init(void);
void app_cursors_update(void);
void app_cursors_free(void);

App app;

#endif
