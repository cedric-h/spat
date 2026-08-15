#ifndef __RECT_IMPL
#define __RECT_IMPL
#include <SDL3/SDL.h>

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

/* cuts away all sides by a depth of "a"  */
SDL_Rect rect_shrink(SDL_Rect rect, float a);

/* grows all sides by a depth of "a"  */
SDL_Rect rect_inflate(SDL_Rect rect, float a);

/* returns a new rect where "child" is in the center of "parent" */
SDL_Rect rect_centered_in(SDL_Rect parent, SDL_Rect child);

/* "clamps" point inside rect */
SDL_Point rect_clamp_point(SDL_Rect rect, SDL_Point p);

#endif
