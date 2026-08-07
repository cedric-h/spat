#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <MiniFB.h>

static struct {
    uint32_t  size_x;
    uint32_t  size_y;
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
    if (scale <= 0.0f) {
        mfb_set_viewport(window, 0, 0, 1, 1);
        return;
    }

    float sx = scale * spat.size_x;
    float sy = scale * spat.size_y;
    float x = (size_x - sx) * 0.5;
    float y = (size_y - sy) * 0.5;
    mfb_set_viewport(window, x, y, sx, sy);
}

int main() {
    mfb_set_log_level(0);

    struct mfb_window *window = mfb_open_ex(
        "spat",
        spat.size_x*5,
        spat.size_y*5,
        MFB_WF_RESIZABLE
    );
    if (!window) return -1;

    spat.buf = (uint32_t *) malloc(spat.size_x * spat.size_y * 4);
    mfb_set_resize_callback(window, spat_resize);

    uint32_t accumulator = 0;
    mfb_update_state state;
    do {
        accumulator++;
        for (uint32_t i = 0; i < spat.size_x * spat.size_y; ++i) {
            uint32_t x = i % spat.size_x;
            uint32_t y = i / spat.size_x;

            uint8_t check = (x ^ y)%2 ? 0xff : 0;
            spat.buf[i] = MFB_ARGB(0xff, check, check, accumulator % 0xff);
        }

        state = mfb_update_ex(window, spat.buf, spat.size_x, spat.size_y);
        if (state != MFB_STATE_OK) {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));

    return 0;
}
