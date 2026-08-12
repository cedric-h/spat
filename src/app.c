#include "app.h"

App app = (App) {0};

static struct {
    SDL_Cursor *cursors[app_Cursor_COUNT];
} __app;

SDL_SystemCursor app_cursor_to_sdl_system_cursor[app_Cursor_COUNT] = {
    [app_Cursor_Default ] = SDL_SYSTEM_CURSOR_DEFAULT,
    [app_Cursor_Pointer ] = SDL_SYSTEM_CURSOR_POINTER,
    [app_Cursor_Move    ] = SDL_SYSTEM_CURSOR_MOVE,
    [app_Cursor_Grab    ] = SDL_SYSTEM_CURSOR_GRAB,
    [app_Cursor_Grabbing] = SDL_SYSTEM_CURSOR_GRABBING,
};

void app_cursors_init() {
    for (int i = 0; i < app_Cursor_COUNT; i++)
        __app.cursors[i] = SDL_CreateSystemCursor(app_cursor_to_sdl_system_cursor[i]);
}
void app_cursors_update() {
    SDL_SetCursor(__app.cursors[app.cursor]);
    app.cursor = app_Cursor_Default;
}
void app_cursors_free() {
    for (int i = 0; i < app_Cursor_COUNT; i++)
        SDL_DestroyCursor(__app.cursors[i]);
}

