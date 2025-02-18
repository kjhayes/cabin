
#include "term.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct terminal_data terminal_data = { 0 };

int
init_terminal(
        FILE *input_file,
        size_t width,
        size_t height)
{
    struct terminal_data *tdata = &terminal_data;
    memset(tdata, 0, sizeof(struct terminal_data));

    tdata->width = width;
    tdata->height = height;

    tdata->redraw_buffer = malloc(width*height*sizeof(unsigned char));
    if(tdata->redraw_buffer == NULL) {
        return -EINVAL;
    }
    memset(tdata->redraw_buffer, 1, width*height*sizeof(unsigned char));

    tdata->character_buffer = malloc(width*height*sizeof(char));
    if(tdata->character_buffer == NULL) {
        free(tdata->redraw_buffer);
        return -EINVAL;
    }
    memset(tdata->character_buffer, ' ', width*height*sizeof(char));

    tdata->fg_color_buffer = malloc(width*height*sizeof(color_t));
    if(tdata->fg_color_buffer == NULL) {
        free(tdata->character_buffer);
        free(tdata->redraw_buffer);
        return -EINVAL;
    }
    memset(tdata->fg_color_buffer, 0xFF, width*height*sizeof(color_t));

    tdata->bg_color_buffer = malloc(width*height*sizeof(color_t));
    if(tdata->bg_color_buffer == NULL) {
        free(tdata->fg_color_buffer);
        free(tdata->character_buffer);
        free(tdata->redraw_buffer);
        return -EINVAL;
    }
    memset(tdata->bg_color_buffer, 0x00, width*height*sizeof(color_t));

    tdata->running = 1;
    tdata->input_file = input_file;
    tdata->cursor_x = 0;
    tdata->cursor_y = 0;
    tdata->cur_fg_color.r = 0xFF;
    tdata->cur_fg_color.g = 0xFF;
    tdata->cur_fg_color.b = 0xFF;
    tdata->cur_fg_color.a = 0xFF;
    tdata->cur_bg_color.r = 0x00;
    tdata->cur_bg_color.g = 0x00;
    tdata->cur_bg_color.b = 0x00;
    tdata->cur_bg_color.a = 0x00;
    tdata->tabsize = 4;

    return 0;
}

void
deinit_terminal(void)
{
    struct terminal_data *tdata = &terminal_data;
    free(tdata->bg_color_buffer);
    free(tdata->fg_color_buffer);
    free(tdata->character_buffer);
    free(tdata->redraw_buffer);
}

static inline void
mark_redraw(struct terminal_data *tdata, size_t __x, size_t __y) {
    terminal_data.redraw_buffer[__x + (__y * tdata->width)] = 1;
}

static inline void
mark_redraw_all(struct terminal_data *tdata) {
    memset(terminal_data.redraw_buffer, 1, tdata->width * tdata->height * sizeof(char));
}

static inline void
newline(struct terminal_data *tdata)
{
    tdata->cursor_y++;
    if(tdata->cursor_y >= tdata->height) {
        memmove(tdata->character_buffer, tdata->character_buffer + tdata->width, tdata->width*(tdata->height-1) * sizeof(char));
        memmove(tdata->fg_color_buffer, tdata->fg_color_buffer + tdata->width, tdata->width*(tdata->height-1) * sizeof(color_t));
        memmove(tdata->bg_color_buffer, tdata->bg_color_buffer + tdata->width, tdata->width*(tdata->height-1) * sizeof(color_t));
        for(size_t __i = 0; __i < tdata->width; __i++) {
            tdata->character_buffer[__i + (tdata->width*(tdata->height-1))] = ' ';
            tdata->fg_color_buffer[__i + (tdata->width*(tdata->height-1))].data = tdata->cur_fg_color.data;
            tdata->bg_color_buffer[__i + (tdata->width*(tdata->height-1))].data = tdata->cur_bg_color.data;
        }
        tdata->cursor_y = tdata->height-1;
        mark_redraw_all(tdata);
    }
}

static inline void
advance_cursor(struct terminal_data *tdata)
{
    tdata->cursor_x++;
    mark_redraw(tdata, tdata->cursor_x-1, tdata->cursor_y);
    if(tdata->cursor_x >= tdata->width) {
        tdata->cursor_x = 0;
        newline(tdata);
    }
    mark_redraw(tdata, tdata->cursor_x, tdata->cursor_y);
}

static inline void
put_at_cursor(struct terminal_data *tdata, char __c)
{
    tdata->character_buffer[tdata->cursor_x + (tdata->cursor_y*tdata->width)] = __c;
    tdata->fg_color_buffer[tdata->cursor_x + (tdata->cursor_y*tdata->width)] = tdata->cur_fg_color;
    tdata->bg_color_buffer[tdata->cursor_x + (tdata->cursor_y*tdata->width)] = tdata->cur_bg_color;
    mark_redraw(tdata, tdata->cursor_x, tdata->cursor_y);
}

static inline void
clear_cursor_to_end_of_screen(struct terminal_data *tdata) {
    size_t cursor_offset = tdata->cursor_x + (tdata->cursor_y*tdata->width);
    size_t room_after = (tdata->width * tdata->height) - cursor_offset;
    memset(tdata->character_buffer + cursor_offset, ' ', room_after);
    memset(tdata->redraw_buffer + cursor_offset, 1, room_after);
}
static inline void
clear_cursor_to_beginning_of_screen(struct terminal_data *tdata)
{
    size_t cursor_offset = tdata->cursor_x + (tdata->cursor_y*tdata->width);
    memset(tdata->character_buffer, ' ', cursor_offset+1);
    memset(tdata->redraw_buffer, 1, cursor_offset+1);
}
static inline void
clear_entire_screen(struct terminal_data *tdata) {
    memset(tdata->character_buffer, ' ', tdata->width*tdata->height);
    memset(tdata->redraw_buffer, 1, tdata->width*tdata->height);
}

static inline void
handle_csi(struct terminal_data *tdata)
{
    char c;

    size_t num_parameter_bytes = 0;
    char parameter_bytes[16+1];
    c = fgetc(tdata->input_file);
    while(num_parameter_bytes < 16) {
        if(0x30 <= c && c <= 0x3F) {
            parameter_bytes[num_parameter_bytes] = c;
            num_parameter_bytes++;
            c = fgetc(tdata->input_file);
        } else {
            break;
        }
    }
   
    size_t num_intermediate_bytes = 0;
    char intermediate_bytes[16+1];
    while(num_intermediate_bytes < 16) {
        if(0x20 <= c && c <= 0x2F) {
            intermediate_bytes[num_intermediate_bytes] = c;
            num_intermediate_bytes++;
            c = fgetc(tdata->input_file);
        } else {
            break;
        }
    }

    if(!(0x40 <= c && c <= 0x7E)) {
        // Missing Terminator
        put_at_cursor(tdata, '?');
        advance_cursor(tdata);
        return;
    }

    char terminator = c;

    int n;

#define SINGLE_PARAMETER_NUM(__default) \
    do {\
        if(num_parameter_bytes > 0) {\
            parameter_bytes[num_parameter_bytes] = '\0';\
            n = atoi(parameter_bytes);\
        } else {\
            n = __default;\
        }\
    } while(0)

    switch(terminator) {
        case 'J':
            // Erase in display
            SINGLE_PARAMETER_NUM(0);
            switch(n) {
                case 0: clear_cursor_to_end_of_screen(tdata); return;
                case 1: clear_cursor_to_beginning_of_screen(tdata); return;
                case 2: clear_entire_screen(tdata); tdata->cursor_x = 0; tdata->cursor_y = 0; return;
                case 3: clear_entire_screen(tdata); /* Note we should also erase any "scrollback */ return;
                default: put_at_cursor(tdata, '?'); advance_cursor(tdata); return;
            }
    }
}

static inline void
handle_escape(struct terminal_data *tdata)
{
    char c = fgetc(tdata->input_file);
    switch(c) {
        case '[':
            return handle_csi(tdata);
        default:
            put_at_cursor(tdata, '?');
            advance_cursor(tdata);
            break;
    }
}

int
run_terminal(void)
{
    struct terminal_data *tdata = &terminal_data;

    mark_redraw_all(tdata);
    while(tdata->running) {
        char c = fgetc(tdata->input_file);
        switch(c) {
            case '\r':
                tdata->cursor_x = 0;
                break;
            case '\n':
                tdata->cursor_x = 0;
                newline(tdata);
                break;
            case '\b':
                if(tdata->cursor_x > 0) {
                    tdata->cursor_x--;
                    mark_redraw(tdata, tdata->cursor_x+1, tdata->cursor_y);
                    mark_redraw(tdata, tdata->cursor_x, tdata->cursor_y);
                }
                break;
            case '\t':
                for(size_t i = 0; i < tdata->tabsize; i++) {
                    put_at_cursor(tdata, ' ');
                    advance_cursor(tdata);
                    if(tdata->cursor_x % tdata->tabsize == 0) {
                        break;
                    }
                }
                break;
            case 0x1B:
                handle_escape(tdata);
                break;
            default:
                put_at_cursor(tdata, c);
                advance_cursor(tdata);
                break;
        }
    }

    return 0;
}
