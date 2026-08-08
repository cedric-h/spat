SDL's software rasterizer compares a whole command queue and then executes it in
`SW_RunCommandQueue` - see `vendored/SDL/src/render/software/SDL_render_sw.c`

A lot of these simply call out to `SDL_surface.h` methods like `SDL_BlitSurfaceScaled`
