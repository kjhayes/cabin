#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "loadpng.h"

// Function to load PNG file into RGBA buffer
PNGImage* load_png_rgba(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }

    // Check if file is a PNG
    png_byte header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Error: %s is not a PNG file\n", filename);
        fclose(fp);
        return NULL;
    }

    // Create PNG structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error: png_create_read_struct failed\n");
        fclose(fp);
        return NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error: png_create_info_struct failed\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return NULL;
    }

    // Set error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error: libpng error occurred\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    // Initialize PNG I/O
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8); // We already read 8 bytes

    // Read PNG info
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // Transform PNG to RGBA format
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    // Update info structure after transformations
    png_read_update_info(png_ptr, info_ptr);

    // Allocate memory for image data
    PNGImage* image = malloc(sizeof(PNGImage));
    if (!image) {
        fprintf(stderr, "Error: Cannot allocate memory for image structure\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->data = malloc(width * height * 4); // 4 bytes per pixel (RGBA)
    if (!image->data) {
        fprintf(stderr, "Error: Cannot allocate memory for image data\n");
        free(image);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    // Allocate row pointers
    png_bytep* row_pointers = malloc(height * sizeof(png_bytep));
    if (!row_pointers) {
        fprintf(stderr, "Error: Cannot allocate memory for row pointers\n");
        free(image->data);
        free(image);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    // Set up row pointers to point into the image data buffer
    for (int y = 0; y < height; y++) {
        row_pointers[y] = image->data + y * width * 4;
    }

    // Read the image data
    png_read_image(png_ptr, row_pointers);

    // Clean up
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return image;
}

// Function to free the image data
void free_png_image(PNGImage* image) {
    if (image) {
        if (image->data) {
            free(image->data);
        }
        free(image);
    }
}
