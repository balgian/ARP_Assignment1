#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <read_fd> <write_fd>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int read_fd = atoi(argv[1]);
    int write_fd = atoi(argv[2]);

    // Read the obstacles positions
    int xMax, yMax;
    int x_obst, y_obst;
    char info[22];
    int *pos_obst = NULL;
    int size = 0;
    do {
        if (read(read_fd, &info, sizeof(info)) == -1) {
            perror("read");
            return EXIT_FAILURE;
        }
        if (sscanf(info, "%d,%d", &x_obst, &y_obst) == 2) {
            pos_obst = (int *) realloc(pos_obst, (size+2)*sizeof(int));
            if (!pos_obst) {
                perror("realloc");
                return EXIT_FAILURE;
            }
            pos_obst[size] = x_obst;
            pos_obst[size+1] = y_obst;
            size += 2;
        }
    } while (strcmp(info, "e") != 0);

    // Gives the target positions cheching if they are not in the obstacles positions
    while(1){
        if (read(read_fd, &info, sizeof(info)) == -1){
            perror("read");
            return EXIT_FAILURE;
        }
        sscanf(info, "%d,%d", &xMax, &yMax);
        snprintf(info, sizeof(info), "s");
        write(write_fd, info, sizeof(info));
        for (int y = 1; y < yMax-1; y++) {
            for (int x = 1; x < xMax-5; x++) {
                int found = 0;
                for (int i = 0; i < size; i += 2) {
                    if (pos_obst[i] == x && pos_obst[i+1] == y) {
                        found = 1;
                        break;
                    }
                }
                if (rand() % 100 < 0.0005 && !found) {
                    snprintf(info, sizeof(info), "%d,%d", x, y);
                    if (write(write_fd, info, sizeof(info)) == -1) {
                        perror("write");
                        return EXIT_FAILURE;
                    }
                }
            }
        }
        snprintf(info, sizeof(info), "e");
        write(write_fd, info, sizeof(info));
    }
    return EXIT_SUCCESS;
}