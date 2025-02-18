
#include "term.h"
#include "font.h"
#include "kanawha/kfb.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int
run_renderer(
        struct terminal_data *tdata,
        struct font_data *fdata,
        struct kfb_framebuffer *fb,
        int layer) {

#define PIXEL_SIZE 4
#define PIXEL_WIDTH (tdata->width * fdata->width)
#define PIXEL_HEIGHT (tdata->height * fdata->height)

    color_t *pixel_buffer = malloc(PIXEL_SIZE * PIXEL_WIDTH * PIXEL_HEIGHT);

    if(pixel_buffer == NULL) {
        kfb_unload_framebuffer(fb);
        fprintf(stderr, "Failed to allocate framebuffer memory!\n");
        exit(EXIT_FAILURE);
    }
    memset(pixel_buffer, 0, PIXEL_SIZE * PIXEL_WIDTH * PIXEL_HEIGHT);

    struct kfb_image image = {
        .format = FB_LAYER_FORMAT_RGBA32,
        .order = FB_LAYER_ORDER_ROW_MAJOR,
        .resx = PIXEL_WIDTH,
        .resy = PIXEL_HEIGHT,
        .offset = 0,
        .stride = PIXEL_SIZE,
        .data_size = PIXEL_SIZE * PIXEL_HEIGHT * PIXEL_WIDTH,
        .data = (void*)pixel_buffer,
    };

#define RENDER_GLYPH(x,y)\
    do { \
    size_t layer_width = fb->current_mode_info->layer_infos[layer].width;\
    size_t layer_height = fb->current_mode_info->layer_infos[layer].height;\
    char c = terminal_data.character_buffer[x + (y * tdata->width)];\
    struct kfb_image *img = fdata->glyphs[c];\
    size_t offset_x = (x*layer_width)/tdata->width;\
    size_t offset_y = (y*layer_height)/tdata->height;\
    kfb_blit_image_onto_layer(\
            fb,\
            layer,\
            img,\
            offset_x, offset_y,\
            layer_width/tdata->width, layer_height/tdata->height \
            );\
    } while(0)

#define RENDER_ALL() \
    do {\
    for(size_t y = 0; y < tdata->height; y++) {\
        for(size_t x = 0; x < tdata->width; x++) {\
            if(terminal_data.redraw_buffer[x + (y*tdata->width)]) {\
                render_changed = 1;\
                RENDER_GLYPH(x,y);\
                terminal_data.redraw_buffer[x + (y*tdata->width)] = 0;\
            }\
        }\
    }\
    } while(0)

    int render_changed = 0;
    while(tdata->running) {
        RENDER_ALL();
        if(render_changed) {
            kfb_flush_framebuffer(fb);
        }
    }

    return 0;
}
