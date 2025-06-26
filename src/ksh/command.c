
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "command.h"

int
command_state_init(
        struct command_state *state)
{
    state->data_len = 0;
    state->cursor_pos = 0;
    state->printed_prefix = 0;

    state->bufsize = 0x1000;
    state->buffer = malloc(state->bufsize);
    if(state->buffer == NULL) {
        state->bufsize = 0x0;
        return -ENOMEM;
    }

    return 0;
}

int
command_state_deinit(
        struct command_state *state)
{
    state->bufsize = 0x0;
    free(state->buffer);
    return 0;
}

int
command_state_tick(
        struct command_state *state,
        int have_char,
        char c)
{
    if(state->data_len == 0 && !state->printed_prefix) {
        char cwd_buffer[64];
        getcwd(cwd_buffer, 64);
        cwd_buffer[63] = '\0';
        printf("%s> ", cwd_buffer);
        state->printed_prefix = 1;
    }

    if(have_char) {
        switch(c) {
            case '\b': // Backspace
                if(state->cursor_pos == 0) {
                    // Cannot backspace further
                } else {
                    size_t room_after = state->data_len - state->cursor_pos;
                    memmove(
                            state->buffer + state->cursor_pos-1,
                            state->buffer + state->cursor_pos,
                            room_after);
                    state->data_len--;
                    state->cursor_pos--;
                }
                putchar('\b');
                putchar(' ');
                putchar('\b');
                return 0;
        }

        // Regular Character Push
        if(state->data_len >= state->bufsize-1) {
            return -ENOMEM;
        }

        size_t room_after_cursor = state->data_len - state->cursor_pos;
        if(room_after_cursor > 0) {
            memmove(state->buffer + state->cursor_pos + 1,
                    state->buffer + state->cursor_pos,
                    room_after_cursor);
        }
        state->buffer[state->cursor_pos] = c;
        state->data_len++;
        state->cursor_pos++;

        // We don't give a way to move the cursor for now,
        // so this should be fine for mirroring
        putchar(c);
    }

    return 0;
}

char *
command_state_complete_line(
        struct command_state *state)
{
    state->buffer[state->data_len] = '\0';
    state->data_len = 0;
    state->cursor_pos = 0;
    state->printed_prefix = 0;
    return state->buffer;
}

