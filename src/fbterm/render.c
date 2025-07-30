
#include "term.h"
#include "font.h"
#include "kanawha/kfb.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int
run_renderer(
        struct terminal_data *tdata,
        struct font_data *fdata,
        struct kfb_framebuffer *fb,
        int layer) {

#define RENDER_GLYPH(x,y)\
    do { \
    size_t layer_width = fb->current_mode_info->layer_infos[layer].width;\
    size_t layer_height = fb->current_mode_info->layer_infos[layer].height;\
    char c = terminal_data.character_buffer[x + (y * tdata->width)];\
    color_t fg_color = terminal_data.fg_color_buffer[x + (y * tdata->width)];\
    color_t bg_color = terminal_data.bg_color_buffer[x + (y * tdata->width)];\
    struct kfb_image *fg_img = fdata->glyphs[c].fg;\
    struct kfb_image *bg_img = fdata->glyphs[c].bg;\
    if(x == tdata->cursor_x && y == tdata->cursor_y) {\
        color_t temp = fg_color;\
        fg_color = bg_color;\
        bg_color = temp;\
    }\
    size_t offset_x = (x*layer_width)/tdata->width;\
    size_t offset_y = (y*layer_height)/tdata->height;\
    kfb_rgba_t kfb_fg_color = {\
        .r = fg_color.r,\
        .g = fg_color.g,\
        .b = fg_color.b,\
        .a = fg_color.a,\
    };\
    kfb_rgba_t kfb_bg_color = {\
        .r = bg_color.r,\
        .g = bg_color.g,\
        .b = bg_color.b,\
        .a = bg_color.a,\
    };\
    kfb_blit_image_brightness_as_color_onto_layer(\
            fb,\
            layer,\
            fg_img,\
            offset_x, offset_y,\
            layer_width/tdata->width, layer_height/tdata->height, \
            kfb_fg_color);\
    kfb_blit_image_brightness_as_color_onto_layer(\
            fb,\
            layer,\
            bg_img,\
            offset_x, offset_y,\
            layer_width/tdata->width, layer_height/tdata->height, \
            kfb_bg_color);\
    } while(0)

#define RENDER_ALL(__FORCE) \
    do {\
    for(size_t y = 0; y < tdata->height; y++) {\
        for(size_t x = 0; x < tdata->width; x++) {\
            if(__FORCE || terminal_data.redraw_buffer[x + (y*tdata->width)]) {\
                render_changed = 1;\
                RENDER_GLYPH(x,y);\
                terminal_data.redraw_buffer[x + (y*tdata->width)] = 0;\
            }\
        }\
    }\
    } while(0)

    volatile int render_changed = 0;

#define RENDER_DELAY_MS 1
#define FORCE_FLUSH_AFTER 10

    size_t force_timer = 0;

    int force = 1;

    size_t layer_width = fb->current_mode_info->layer_infos[layer].width;
    size_t layer_height = fb->current_mode_info->layer_infos[layer].height;
    fprintf(tdata->log_file, "Screen Dimensions (%ld, %ld)\n",
            layer_width,
            layer_height);
 
    while(tdata->running) {
        RENDER_ALL(force);
        if(render_changed) {
            kfb_flush_framebuffer(fb);
            render_changed = 0;
            force_timer = 0;
        }
        
        usleep(1000 * RENDER_DELAY_MS);

        force_timer += RENDER_DELAY_MS;
        if(force_timer >= FORCE_FLUSH_AFTER) {
            render_changed = 1;
            force_timer = 0;
            force = 1;
        } else {
            force = 0;
        }
    }

    return 0;
}
