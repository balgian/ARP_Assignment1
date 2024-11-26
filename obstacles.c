#include <stdlib.h>

int main (int argc, char *argv[]) {
    void refresh_game_screen(const int xMax, const int yMax) {
        // Clear the inside of the box
        for (int y = 1; y < yMax-1; y++) {
            for (int x = 1; x < xMax-5; x++) {
                mvaddch(y, x, ' ');
            }
        }
    }
    return EXIT_SUCCESS;
}