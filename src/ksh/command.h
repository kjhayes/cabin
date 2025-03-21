#ifndef __CABIN_KSH__COMMAND_H__
#define __CABIN_KSH__COMMAND_H__

struct command_string {
    char *str;

    struct command_string *next;
    struct command_string *prev;
};

#endif
