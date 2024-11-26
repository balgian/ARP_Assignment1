#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

int main(int argc, char *argv[]) {
    initscr();
    nodelay(stdscr, TRUE);
    do {
        int c = getch();
        if (c != ERR) {
            switch (c) {
                case 'w': // Up Left
                case 'e': // Up
                case 'r': // Up Right or Reset
                case 's': // Left or Suspend
                case 'd': // Breake
                case 'f': // Right
                case 'x': // Down Left
                case 'c': // Down
                case 'v': // Down Right
                case 'p': // Pause
                case 'q': // Quit
                    printw("i%c", c);
                break;
                default:
                    break;
            }
        }
        usleep(10000); // Sleep for 0.01 seconds
    } while (1);
    endwin();
    return EXIT_SUCCESS;
}