#include "rect.h"
#include "app.h"

SDL_Rect rect_cut_left(SDL_Rect *rect, float a) {
    SDL_Rect ret = *rect;
    ret.w = a;

    rect->x += a;
    rect->w -= a;
    return ret;
}

SDL_Rect rect_cut_top(SDL_Rect *rect, float a) {
    SDL_Rect ret = *rect;
    ret.h = a;

    rect->y += a;
    rect->h -= a;
    return ret;
}

SDL_Rect rect_cut_right(SDL_Rect *rect, float a) {
    SDL_Rect ret = *rect;
    ret.x += rect->w - a;
    ret.w = a;

    rect->w -= a;
    return ret;
}

SDL_Rect rect_cut_bottom(SDL_Rect *rect, float a) {
    SDL_Rect ret = *rect;
    ret.y += rect->h - a;
    ret.h = a;

    rect->h -= a;
    return ret;
}

SDL_Rect rect_shrink(SDL_Rect rect, float a) {
    rect.x += a;
    rect.y += a;
    rect.w -= a*2;
    rect.h -= a*2;
    return rect;
}

SDL_Rect rect_inflate(SDL_Rect rect, float a) {
    rect.x -= a;
    rect.y -= a;
    rect.w += a*2;
    rect.h += a*2;
    return rect;
}

SDL_Rect rect_centered_in(SDL_Rect parent, SDL_Rect child) {
    return (SDL_Rect) {
        .x = parent.x + (parent.w - child.w)/2,
        .y = parent.y + (parent.h - child.h)/2,
        .w = child.w,
        .h = child.h,
    };
}

SDL_Point rect_clamp_point(SDL_Rect r, SDL_Point p) {
    float min_x = r.x, min_y = r.y;
    float max_x = r.x + r.w, max_y = r.y + r.h;
    return (SDL_Point) {
        .x = spat_clamp(min_x, max_x, p.x),
        .y = spat_clamp(min_y, max_y, p.y),
    };
}
