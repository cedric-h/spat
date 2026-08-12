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

void rect_shrink(SDL_Rect *rect, float a) {
    rect->x += a;
    rect->y += a;
    rect->w -= a*2;
    rect->h -= a*2;
}
