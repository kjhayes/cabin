
#include <ncurses/curses.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses/ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"


#define MAX_CMD_LEN 32


WINDOW *status;

void enter_insert_mode(WINDOW *edit, int y, int x);
void search(WINDOW *command);
void enter_command_mode(WINDOW *command);
void clear_command(WINDOW *command);


int main(int argc, const char **argv)
{
    // int fd = open("./frankenstein.txt", O_RDONLY);
    // if (fd == -1)
    //     errExit("Error opening file");

    if(argc < 2) {
        errExit("Need to provide file name!\n");
    }

    int c;
    long len;

    FILE *fp = fopen(argv[1], "r+");
    if (fp == NULL)
        errExit("Error opening file");
    while ((c = getc(fp)) != EOF)
        ++len;
    char *buffer = (char *)malloc(1000 * sizeof(char));

    rewind(fp);
    char *temp = buffer;
    for (int i = 0; i < 1000; ++i)
        *temp++ = (char) getc(fp);


    fclose(fp);

    initscr();
    noecho();

    if (has_colors())
    {
        start_color();

        init_pair(1, COLOR_BLACK, COLOR_WHITE);
    }

    if (NCURSES_MOUSE_VERSION > 0)
    {
        mousemask(BUTTON1_CLICKED, NULL);
    }

    int y, x;
    WINDOW *edit = newwin(LINES - 3, COLS, 0, 0);
    keypad(edit, TRUE);
    waddstr(edit, buffer);
    wmove(edit, 0, 0);
    getyx(edit, y, x);
    wrefresh(edit);

    status = newwin(1, COLS, LINES - 2, 0);
    wbkgd(status, COLOR_PAIR(1));

    mvwprintw(status, 0, 0, "%d,%d", y, x);
    wrefresh(status);
    wrefresh(edit);

    WINDOW *command = newwin(1, COLS, LINES - 1, 0);

    int ch;
    MEVENT mouse_event;
    while ((ch = wgetch(edit)))
    {
        switch (ch)
        {
            case '0':
                wmove(edit, y, 0);
                break;
            case '$':
                wmove(edit, y, COLS - 1);
                break;
            case '/':
                search(command);
                break;
            case KEY_LEFT:
            case 'h':
                wmove(edit, y, x - 1);
                break;
            case KEY_DOWN:
            case 'j':
                wmove(edit, y + 1, x);
                break;
            case KEY_UP:
            case 'k':
                wmove(edit, y - 1, x);
                break;
            case KEY_RIGHT:
            case 'l':
                wmove(edit, y, x + 1);
                break;
            case 'C':
                wclrtoeol(edit);
                enter_insert_mode(edit, y, x);
                break;
            case 'D':
                wclrtoeol(edit);
                break;
            case 'x':
                wdelch(edit);
                break;
            case 'X':
                if (x != 0)
                {
                    wmove(edit, y, --x);
                    wdelch(edit);
                }
                break;
            case 'a':
                wmove(edit, y, ++x);
                enter_insert_mode(edit, y, x);
                break;
            case 'I':
                wmove(edit, y, x=0);
            case 'i':
                enter_insert_mode(edit, y, x);
                break;
            case ':':
                enter_command_mode(command);
                break;
            case KEY_MOUSE:
                getmouse(&mouse_event);
                wmove(edit, mouse_event.y, mouse_event.x);
                break;
            default:
                break;

        }
        getyx(edit, y, x);
        mvwprintw(status, 0, 0, "%d,%d", y, x);
        wclrtoeol(status);
        wrefresh(status);
        wrefresh(edit);
    }

    endwin();
    // if (fd == -1)
    //     errExit("Error closing file");
    free(buffer);
}

void enter_insert_mode(WINDOW *edit, int y, int x)
{
    int ch;
    MEVENT mouse_event;

    while ((ch = wgetch(edit)) != 27)
    {
        switch (ch)
        {
            case KEY_BACKSPACE:
            case 8:
            case 127:
                if (x != 0)
                {
                    wmove(edit, y, --x);
                    wdelch(edit);
                }
                break;
            case '\n':
                wmove(edit, ++y, x = 0);
                winsertln(edit);
                break;
            case KEY_MOUSE:
                getmouse(&mouse_event);
                y = mouse_event.y;
                x = mouse_event.x;
                wmove(edit, mouse_event.y, mouse_event.x);
                break;
            default:
                winsch(edit, ch);
                wmove(edit, y, ++x);
                wrefresh(edit);
                break;
        }
        getyx(edit, y, x);
        mvwprintw(status, 0, 0, "%d,%d", y, x);
        wclrtoeol(status);
        wrefresh(status);
        wrefresh(edit);
    }
}

void search(WINDOW *command)
{
    char search_term[MAX_CMD_LEN];
    int y, x;

    echo();
    clear_command(command);
    mvwaddstr(command, 0, 0, "/");

    int ch;
    for (int i = 0; i < MAX_CMD_LEN; ++i)
    {
        switch (ch = wgetch(command))
        {
            case '\n':
                clear_command(command);
                wprintw(command, "Searching for '%s'", search_term);
                wrefresh(command);
                goto exiting_search;
            case 27:
                clear_command(command);
                goto exiting_search;
            case 127:
                getyx(command, y, x);
                x -= 2;
                wmove(command, y, x);
                wclrtoeol(command);

                if (x == 1)
                {
                    clear_command(command);
                    goto exiting_search;
                } else if (x > 1) {
                    search_term[--i] = '\0';
                    --i;
                    wmove(command, y, --x);
                    wdelch(command);
                }

                break;
            default:
                search_term[i] = ch;
                break;
        }
    }

    exiting_search:
    noecho();
}

void enter_command_mode(WINDOW *command)
{
    char cmd[MAX_CMD_LEN];
    int y, x;

    echo();
    clear_command(command);
    mvwaddstr(command, 0, 0, ":");

    int ch;
    for (int i = 0; i < MAX_CMD_LEN; ++i)
    {
        switch (ch = wgetch(command))
        {
            case '\n':
                if (strcmp(cmd, "q") == 0)
                {
                    endwin();
                    exit(EXIT_SUCCESS);
                } else {
                    clear_command(command);
                    goto exiting_command_mode;
                }
            case 27:
                    goto exiting_command_mode;
            case 127:
                getyx(command, y, x);
                x -= 2;
                wmove(command, y, x);
                wclrtoeol(command);

                if (x == 1)
                {
                    clear_command(command);
                    goto exiting_command_mode;
                } else if (x > 1) {
                    cmd[--i] = '\0';
                    --i;
                    wmove(command, y, --x);
                    wdelch(command);
                }

                break;
            default:
                cmd[i] = ch;
                break;
        }
    }

    exiting_command_mode:
    clear_command(command);
    noecho();
}

void clear_command(WINDOW *command)
{
    wmove(command, 0, 0);
    wclrtoeol(command);
    wrefresh(command);
}

