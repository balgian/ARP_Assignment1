#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <read_fd> <write_fd>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int read_fd = atoi(argv[1]);
    int write_fd = atoi(argv[2]);

    do {
        char c = getchar();
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
                write(write_fd, &c, 1);
                break;
            default:
                break;
        }
        usleep(1000); // Sleep for 0.01 seconds
    } while (1);
    close(read_fd);
    return EXIT_SUCCESS;
}