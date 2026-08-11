#ifndef __RECT_IMPL
#define __RECT_IMPL

/**
 * see https://archive.ph/Gglet (archive of https://halt.software/dead-simple-layouts/) for rationale
 **/

/* cuts off the left side of "rect" at a depth of "a" returns the sliced off bit as a new rect */
SDL_Rect rect_cut_left(SDL_Rect *rect, float a);

/* cuts off the top side of "rect" at a depth of "a" and returns the sliced off bit as a new rect */
SDL_Rect rect_cut_top(SDL_Rect *rect, float a);

/* cuts off the right side of "rect" at a depth of "a" and returns the sliced off bit as a new rect */
SDL_Rect rect_cut_right(SDL_Rect *rect, float a);

/* cuts off the bottom side of "rect" at a depth of "a" and returns the sliced off bit as a new rect */
SDL_Rect rect_cut_bottom(SDL_Rect *rect, float a);

#endif
