
#include <ncurses/ncurses.h>

int main() {
    int row, col;

    initscr();                       // Initialize ncurses
    getmaxyx(stdscr, row, col);      // Get the dimensions of the window
    mvprintw(row / 2, col / 2 - 7, "Hello, ncurses!"); // Print text in the center
    refresh();                       // Refresh the screen to show the changes
    getch();                         // Wait for user input
    endwin();                        // End ncurses

    printf(" %d %d \n", row, col);
    printf(" %d %d \n", row / 2, col / 2 - 7);

    return 0;
}

