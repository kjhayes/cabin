#ifndef __CABIN_FBTERM__TERM_H__
#define __CABIN_FBTERM__TERM_H__

#include "color.h"
#include <stddef.h>
#include <stdio.h>

extern struct terminal_data
{
    // Display Data
    size_t width;
    size_t height;
    unsigned char *redraw_buffer;
    char *character_buffer;
    color_t *fg_color_buffer;
    color_t *bg_color_buffer;

    // State Data
    volatile int running;
    FILE *input_file;

    // Draw Data
    size_t cursor_x;
    size_t cursor_y;
    color_t cur_fg_color;
    color_t cur_bg_color;
    size_t tabsize;

} terminal_data;

// "input" must outlive this terminal
int
init_terminal(FILE *input, size_t width, size_t height);

void
deinit_terminal(void);

int
run_terminal(void);

#endif
