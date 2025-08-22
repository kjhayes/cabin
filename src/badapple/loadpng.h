
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

// Structure to hold image data
typedef struct PNGImage {
    unsigned char* data;
    int width;
    int height;
} PNGImage;

PNGImage* load_png_rgba(const char* filename);
void free_png_image(PNGImage* image);

