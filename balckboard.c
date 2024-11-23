#include <stdio.h>
#include <ncurses.h>

void refresh_game_screen(const int xMax, const int yMax) {
    // Clear the inside of the box
    for (int y = 1; y < yMax-1; y++) {
        for (int x = 1; x < xMax-5; x++) {
            mvaddch(y, x, ' ');
        }
    }
}

void save_game_screen(const int xMax, const int yMax) {

}

int main(void) {
    // Open a new ncurses window
    initscr();
    // Deactivate mouse support
    mousemask(0, NULL);
    // Delete the echo of the keys
    noecho();
    // Delete the cursor highlight
    curs_set(0);
    // Create a border around the whole window
    // Get the maximum x and y coordinates of the window
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    // Print the top border of the window
    for (int i = 0; i < xMax-5; i++) {
        // Place the string '-' at the top of the window at the current x coordinate
        mvaddch(0, i, '-');
        // Place the string '-' at the bottom of the window at the current x coordinate
        mvaddch(yMax-1, i, '-');
    }
    // Print the sides of the window
    for (int i = 1; i < yMax-1; i++) {
        // Place the string '|' at the left of the window at the current y coordinate
        mvaddch(i, 0, '|');
        // Place the string '|' at the right of the window at the current y coordinate
        mvaddch(i, xMax-5, '|');
    }
    // Print the corners of the window
    // Top left
    mvaddch(0, 0, '+');
    // Top right
    mvaddch(0, xMax-5, '+');
    // Bottom left
    mvaddch(yMax-1, 0, '+');
    // Bottom right
    mvaddch(yMax-1, xMax-5, '+');
    // Write the string "Push s to start" at the centre of the window
    int middleX = (xMax-10)/2;
    int middleY = yMax/2;
    mvprintw(middleY, middleX, "Press s to start");
    // Refresh the ncurses window to display the changes
    refresh();
    // Wait for the user to press the key 's'
    while (getch() != 's') {}
    refresh_game_screen(xMax, yMax);
    // Refresh the ncurses window to display the changes
    refresh();
    // Print the top border of the window
    mvaddch(middleY, middleX-10, '+');
    while (true) {
        // Wait for the user to press the key 'q'
        mvprintw(0, xMax - 23, " Press p to pause ");
        if (getch() == 'p') {

            mvprintw(middleY, middleX, "Press q to quit");
            if (getch() == 'q') {
                // TODO: Send the quit to the other processes
                break;
            }
        }
        // TODO: Receive the data from the keyboard manager process
        // TODO: Receive the data from the drone dynamics process
        // TODO: Receive the data from the obstacles process
        // TODO: Receive the data from the targets generator process
        // TODO: Receive the data from the watchdog process
        refresh_game_screen(xMax, yMax);
    }
    // Close the ncurses window
    endwin();
    return 0;
}