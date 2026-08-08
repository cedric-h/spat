#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <MiniFB.h>

#define dbg() __builtin_debugtrap()
#define spat_min(x, y) (((x) < (y)) ? (x) : (y))
#define spat_max(x, y) (((x) > (y)) ? (x) : (y))

#include "font.h"

static struct {
    uint32_t  offset_x, offset_y, size_x, size_y, canvas_scale;
    uint32_t *buf;
} spat = {
    .size_x = 160,
    .size_y =  90,
};

static void spat_resize(struct mfb_window *window, int size_x, int size_y) {
    float scale = floorf(fmin(
        (float)size_x / spat.size_x,
        (float)size_y / spat.size_y
    ));
    spat.canvas_scale = scale;
    if (scale <= 0.0f) {
        mfb_set_viewport(window, 0, 0, 1, 1);
        return;
    }

    float sx = scale * spat.size_x;
    float sy = scale * spat.size_y;
    float x = (size_x - sx) * 0.5;
    float y = (size_y - sy) * 0.5;
    spat.offset_x = x;
    spat.offset_y = y;
    mfb_set_viewport(window, x, y, sx, sy);
}

int main() {
    mfb_set_log_level(0);

    spat.canvas_scale = 5;
    struct mfb_window *window = mfb_open_ex(
        "spat",
        spat.size_x*spat.canvas_scale,
        spat.size_y*spat.canvas_scale,
        MFB_WF_RESIZABLE | MFB_WF_BORDERLESS
    );
    if (!window) return -1;
    spat_resize(window, spat.size_x*spat.canvas_scale, spat.size_y*spat.canvas_scale);

    spat.buf = (uint32_t *) malloc(spat.size_x * spat.size_y * 4);
    mfb_set_resize_callback(window, spat_resize);

    mfb_update_state state;
    do {
        for (uint32_t i = 0; i < spat.size_x * spat.size_y; ++i) {
            uint32_t x = i % spat.size_x;
            uint32_t y = i / spat.size_x;

            spat.buf[i] = (x ^ y)%2 ? MFB_ARGB(0x44, 0x44, 0x44, 0x44) : MFB_ARGB(0x22, 0x22, 0x22, 0x22);
        }

        {
            int mouse_x = mfb_get_mouse_x(window) - (int)spat.offset_x;
            int mouse_y = mfb_get_mouse_y(window) - (int)spat.offset_y;
            uint32_t wx = spat_max(0, spat_min((int)spat.size_x-1, mouse_x / (int)spat.canvas_scale));
            uint32_t wy = spat_max(0, spat_min((int)spat.size_y-1, mouse_y / (int)spat.canvas_scale));
            spat.buf[wy*spat.size_x + wx] = MFB_ARGB(0xff, 0xff, 0xff, 0xff);
        }

        {
            uint32_t dst_x = 15;
            uint32_t dst_y = 15;

            for (char *str = "The five boxing wizards jump quickly"; *str; str++) {
                for (int y = 0; y < 8; y++) {
                    char row = font_data[((size_t)*str)*8 + y];
                    for (int x = 0; x < 8; x++) {
                        bool lit = (row & (1 << (8 - x))) > 0;

                        if (!lit) continue;
                        spat.buf[(dst_y + y)*spat.size_x + (dst_x + x)] = MFB_ARGB(0xff, 0xff, 0xff, 0xff);
                    }
                }

                dst_x += 6;
            }
        }

        state = mfb_update_ex(window, spat.buf, spat.size_x, spat.size_y);
        if (state != MFB_STATE_OK) {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));

    return 0;
}
