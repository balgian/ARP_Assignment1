#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

int main(int argc, char *argv[]) {
  // TODO: Implement the logic to the watchdog's signals
    int pipefd_write = atoi(argv[2]);
    if (pipefd_write == -1) {
        perror("atoi");
        exit(EXIT_FAILURE);
    }
    char out[2];
    do {
        out[0] = 'b';
        out[1] = getch();
        switch (out[1]) {
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
                write(pipefd_write, &out, 2);
                break;
            default:
                break;
        }
    } while (1);
    close(pipefd_write);
    return EXIT_SUCCESS;
}