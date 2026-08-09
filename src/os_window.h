#ifndef __OS_WINDOW_IMPL
#define __OS_WINDOW_IMPL
typedef enum {
    os_window_Mode_Draw,
    os_window_Mode_HitTest,
    os_window_Mode_Capture,
    os_window_Mode_Click,
    os_window_Mode_MouseDown,
} os_window_Mode;
bool os_window_fn(os_window_Mode mode, const SDL_Point *p);
void os_window_set_size(int x, int y);

#define clr_bg     draw_rgba(0x33, 0x33, 0x33, 0xFF)
#define clr_shadow draw_rgba(0x08, 0x08, 0x08, 0xFF)
#define clr_text   draw_rgba(0x66, 0x66, 0x66, 0xFF)
#define clr_black  draw_rgba(0x11, 0x11, 0x11, 0xFF)
#define clr_hilite draw_rgba(0xFF, 0xAA, 0xAA, 0xFF)

typedef enum {
   os_window_Button_ZoomIn,
   os_window_Button_ZoomOut,
   os_window_Button_Quit,
   os_window_Button_NONE,
} os_window_Button;
/* returns true on click */
static bool os_window_button(os_window_Button button, int x, int y, bool enabled);

SDL_HitTestResult os_window_hit_test(SDL_Window *win, const SDL_Point *p, void *data);
bool os_window_event(SDL_Event *event);
#endif
