#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses/ncurses.h>
#include <string.h>

// Map RGB to best-available color index based on COLORS
int rgb_to_indexed(int r, int g, int b, float brightness, int color_levels) {
    r = (int)(r * brightness);
    g = (int)(g * brightness);
    b = (int)(b * brightness);
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;

    int step = 256 / color_levels;
    int ri = r / step;
    int gi = g / step;
    int bi = b / step;

    // Map to a 3D color cube index (fits in COLORS if cube size ≤ color_levels^3)
    return ri * color_levels * color_levels + gi * color_levels + bi;
}

uint8_t* load_tga(const char* filename, int* width, int* height) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    uint8_t header[18];
    fread(header, 1, 18, f);

    *width  = header[12] | (header[13] << 8);
    *height = header[14] | (header[15] << 8);
    int bpp = header[16];

    if (bpp != 24 || header[2] != 2) {
        fprintf(stderr, "Only 24-bit uncompressed TGA supported.\n");
        fclose(f);
        return NULL;
    }

    size_t image_size = (*width) * (*height) * 3;
    uint8_t* data = malloc(image_size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    fread(data, 1, image_size, f);
    fclose(f);
    return data;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s image.tga\n", argv[0]);
        return 1;
    }

    int w, h;
    uint8_t* img = load_tga(argv[1], &w, &h);
    if (!img) return 1;

    initscr();
    start_color();
    use_default_colors();

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Terminal does not support colors.\n");
        free(img);
        return 1;
    }

    int max_colors = COLORS;
    int max_pairs  = COLOR_PAIRS;
    float brightness = 1.5f;

    // Estimate number of RGB levels per channel (e.g. 6x6x6 for 216, 4x4x4 for 64)
    int cube_levels = 1;
    while (cube_levels * cube_levels * cube_levels <= max_colors && cube_levels < 64)
        cube_levels++;

    cube_levels--;

    // Track initialized color pairs
    char* initialized = calloc(max_colors, 1);

    for (int y = 0; y < h && y < LINES; ++y) {
        for (int x = 0; x < w && x < COLS; ++x) {
            int i = ((h - y - 1) * w + x) * 3;
            uint8_t b = img[i];
            uint8_t g = img[i + 1];
            uint8_t r = img[i + 2];

            int color_index = rgb_to_indexed(r, g, b, brightness, cube_levels);
            if (color_index >= max_colors)
                color_index = max_colors - 1;

            if (!initialized[color_index]) {
                init_pair(color_index + 1, color_index, color_index);
                initialized[color_index] = 1;
            }

            if (color_index < max_pairs)
                attron(COLOR_PAIR(color_index + 1));

            mvaddch(y, x, ' ');

            if (color_index < max_pairs)
                attroff(COLOR_PAIR(color_index + 1));
        }
    }

    refresh();
    getch();
    endwin();

    free(img);
    free(initialized);
    return 0;
}

