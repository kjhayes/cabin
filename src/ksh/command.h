#ifndef __KSH_COMMAND_H__
#define __KSH_COMMAND_H__

#include <stdint.h>
#include <stddef.h>

struct command_state
{
    size_t bufsize;
    char *buffer;

    size_t data_len;
    size_t cursor_pos;

    int printed_prefix;
};

int
command_state_init(
        struct command_state *state);

int
command_state_deinit(
        struct command_state *state);

int
command_state_tick(
        struct command_state *state,
        int have_char,
        char c);

char *
command_state_complete_line(
        struct command_state *state);

#endif
