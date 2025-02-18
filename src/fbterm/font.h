#ifndef __CABIN_FBTERM__FONT_H__
#define __CABIN_FBTERM__FONT_H__

#include "color.h"
#include <stddef.h>
#include <stdint.h>
#include "kanawha/kfb.h"

struct font_data
{
    size_t width;
    size_t height;
    size_t error_glyph;
    size_t num_glyphs;
    struct kfb_image **glyphs;
};

struct font_data *
load_font(const char *path);

void
unload_font(struct font_data *fdata);

#endif
