
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"

#define GLYPH_FORMAT FB_LAYER_FORMAT_RGBA32

struct psf_1_header {
    uint8_t magic[2];
    uint8_t flags;
    uint8_t glyph_size;
};

struct font_data *
load_font(const char *path)
{
    FILE *file = fopen(path, "r");
    if(file == NULL) {
        return NULL;
    }

    struct psf_1_header psf_1_header;
    size_t read = fread(&psf_1_header, sizeof(struct psf_1_header), 1, file);
    if(read != 1) {
        fclose(file);
        return NULL;
    }

    if(psf_1_header.magic[0] != 0x36 ||
       psf_1_header.magic[1] != 0x04)
    {
        fclose(file);
        return NULL;
    }

    size_t font_data_size = psf_1_header.glyph_size * 256;
    uint8_t *font_data = malloc(font_data_size);
    if(font_data == NULL) {
        fclose(file);
        return NULL;
    }

    read = fread(font_data, font_data_size, 1, file);
    if(read != 1) {
        free(font_data);
        fclose(file);
        return NULL;
    }

    fclose(file);

    struct font_data *fdata = malloc(sizeof(struct font_data));
    if(fdata == NULL) {
        free(font_data);
        return NULL;
    }
    memset(fdata, 0, sizeof(struct font_data));

    fdata->width = 8;
    fdata->height = psf_1_header.glyph_size;
    fdata->num_glyphs = 256;

    fdata->glyphs = malloc(sizeof(struct image *) * fdata->num_glyphs);
    if(fdata->glyphs == NULL) {
        free(font_data);
        free(fdata);
        return NULL;
    }
    memset(fdata->glyphs, 0, sizeof(struct image *) * fdata->num_glyphs);

    for(size_t i = 0; i < fdata->num_glyphs; i++) {
        struct kfb_image *img = malloc(sizeof(struct kfb_image));
        if(img == NULL) {
            for(size_t fi = 0; fi < i; fi++) {
                free(fdata->glyphs[fi]->data);
                free(fdata->glyphs[fi]);
            }
            free(fdata->glyphs);
            free(fdata);
            free(font_data);
            return NULL;
        }

        color_t *pixel_data = malloc(sizeof(color_t) * fdata->width * fdata->height);
        if(pixel_data == NULL) {
            for(size_t fi = 0; fi < i; fi++) {
                free(fdata->glyphs[fi]->data);
                free(fdata->glyphs[fi]);
            }
            free(img);
            free(fdata->glyphs);
            free(fdata);
            free(font_data);
            return NULL;
        }

        img->resx = fdata->width;
        img->resy = fdata->height;
        img->data = (void*)pixel_data;
        img->format = GLYPH_FORMAT;
        img->order = FB_LAYER_ORDER_ROW_MAJOR;
        img->stride = sizeof(color_t);
        img->offset = 0;
        img->data_size = img->resx * img->resy * sizeof(color_t);

        // Render the image
        uint8_t *glyph = font_data + (i * fdata->height);
        for(size_t fy = 0; fy < fdata->height; fy++) {
            uint8_t bits = glyph[fy];
            for(int fx = 0; fx < 8; fx++) {
                int value = (bits >> (7-fx)) & 1;
                color_t *pixel = &((color_t*)img->data)[fx + (fy*fdata->width)];
                if(value) {
                    pixel->r = 0xFF;
                    pixel->g = 0xFF;
                    pixel->b = 0xFF;
                    pixel->a = 0xFF;
                } else {
                    pixel->r = 0x00;
                    pixel->g = 0x00;
                    pixel->b = 0x00;
                    pixel->a = 0x00;
                }
            }
        }

        fdata->glyphs[i] = img;
    }

    free(font_data);

    return fdata;
}

void
unload_font(struct font_data *fdata)
{
    for(size_t i = 0; i < fdata->num_glyphs; i++) {
        free(fdata->glyphs[i]->data);
        free(fdata->glyphs[i]);
    }
    free(fdata->glyphs);
    free(fdata);
}

