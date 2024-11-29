#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h> // For O_NONBLOCK

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

    // Make a pipe to send/receive data to/from the child processes
    int pipefds [5][2];
    // Create 5 pipes
    for (int i = 0; i < 5; i++) {
        if (pipe(pipefds[i]) == -1) {
            // Print an error message if pipe creation failed
            perror("Pipe");
            // Exit the program with a failure status
            return EXIT_FAILURE;
        }
    }

    // Initialize all processes (drone dynamics; keyboard manager; obstacles; targets generators; watchdog)
    // Each process is responsible for its log data
    // Fork the process to create a new child process
    const pid_t keyboard_manager = fork();
    // Check if fork failed by examining the return value
    if (keyboard_manager == -1) {
        // Print an error message if fork failed
        perror("Keyboard manager");
        // Exit the program with a failure status
        return EXIT_FAILURE;
    }
    if (keyboard_manager == 0) {
        // Close the read end of the pipe in the child process
        close(pipefds[0][0]);
        // Duplicate the write end of the pipe to the standard output
        if (dup2(pipefds[0][1], STDOUT_FILENO) == -1) {
            perror("dup2");
            return EXIT_FAILURE;
        }
        close(pipefds[0][1]);
        // The child process will execute the program "./keyboard_manager" with the argument "keyboard_manager" and the pipe
        // If the return value is not 0, then there was an error
        execl("./keyboard_manager", "keyboard_manager", NULL);
        // Print an error message if execl failed
        perror("Keyboard manager");
        // Exit the program with a failure status
        return EXIT_FAILURE;
    }

    const pid_t obstacles = fork();
    if (obstacles == -1) {
        perror("Obstacles");
        return EXIT_FAILURE;
    }
    if (obstacles == 0) {
        if (dup2(pipefds[2][0], STDIN_FILENO) == -1) {
            perror("dup2");
            return EXIT_FAILURE;
        }
        close(pipefds[2][0]);
        if (dup2(pipefds[2][1], STDOUT_FILENO) == -1) {
            perror("dup2");
            return EXIT_FAILURE;
        }
        close(pipefds[2][1]);

        execl("./obstacles", "obstacles", NULL);
        perror("Obstacles");
        return EXIT_FAILURE;
    }

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
    int middleX = (xMax-5)/2;
    int middleY = (yMax - 1)/2;
    mvprintw(middleY, middleX, "Press s to start");
    mvprintw(middleY + 1, middleX, "Press q to quit");
    // Refresh the ncurses window to display the changes
    refresh();
    // Wait for the user to press the key 's'
    char pushChar[2] = {'\0', '\0'};
    int i = 0;
    do {
        if (read(pipefds[0][0], &pushChar, sizeof(pushChar)) == -1) {
            perror("read");
            close(pipefds[0][0]);
            close(pipefds[0][1]);
            endwin();
            return EXIT_FAILURE;
        }
        if (pushChar[0] == 'i' && pushChar[1] == 'q') {
            pushChar[0] = 'w';
            pushChar[1] = 'q';
            write(pipefds[0][1], &pushChar, sizeof(pushChar));
            endwin();
            return EXIT_SUCCESS;
        }
        refresh();
        usleep(10000); // Sleep for 0.01 seconds
    } while (pushChar[0] != 'i' || pushChar[1] != 's');
    refresh_game_screen(xMax, yMax);
    // Refresh the ncurses window to display the changes
    refresh();

    char info[23];
    snprintf(info, sizeof(info), "o%d,%d", xMax, yMax);
    if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[2][0]);
        close(pipefds[2][1]);
        endwin();
        return EXIT_FAILURE;
    }
    memset(info, '\0', sizeof(info));
    usleep(1000); // Sleep for 1 millisecond

    while (true) {
        if (read(pipefds[2][0], &info, sizeof(info)) == -1) {
            perror("read");
            close(pipefds[2][0]);
            close(pipefds[2][1]);
            endwin();
            return EXIT_FAILURE;
        }
        if (strcmp(info, "os") == 0) {
            mvprintw(middleY, middleX, "done1");
            refresh();
            break;
        }
        if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
            perror("write");
            close(pipefds[2][0]);
            close(pipefds[2][1]);
            endwin();
            return EXIT_FAILURE;
        }
        refresh_game_screen(xMax, yMax);
        refresh();
        usleep(1000); // Sleep for 1 millisecond
    }

    // CI SIAMO
    usleep(100);
    int x, y;
    while (true) {
        if (read(pipefds[2][0], &info, sizeof(info)) == -1) {
            perror("read");
            close(pipefds[2][0]);
            close(pipefds[2][1]);
            endwin();
            return EXIT_FAILURE;
        }
        if (strcmp(info, "oe") == 0) {
            mvprintw(middleY, middleX, "Done3");
            refresh();
            sleep(2);
            break;
        }
        if (scanf(info, "o%d,%d", &x, &y) == 2) {
            mvaddch(x, y, 'o');
            mvprintw(middleY, middleX, "Not a problem");
            refresh();
        }
        else {
            if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
                perror("write");
                close(pipefds[2][0]);
                close(pipefds[2][1]);
                endwin();
                return EXIT_FAILURE;
            }
            mvprintw(middleY, middleX, "There is a problem with the obstacle");
            refresh();
            usleep(1000);
        }
    }

    mvaddch(middleY, middleX-10, '+');

    bool game_pause = false;
    while (true) {
        // Wait for the user to press the key 'q'
        mvprintw(0, xMax - 23, " Press p to pause ");
        if (getch() == 'p') { game_pause = !game_pause;}

        if (game_pause) {
            mvprintw(middleY, middleX, "Press q to quit");
            if (getch() == 'q') {
                // Send the quit to the other processes
                char quitChar = 'q';
                write(pipefds[0][1], &quitChar, sizeof(quitChar));
                break;
            }
            continue;
        }
        // Receive the data from the keyboard manager process

        // TODO: Receive the data from the drone dynamics process
        // TODO: Receive the data from the obstacles process
        // TODO: Receive the data from the targets generator process
        // TODO: Receive the data from the watchdog process
        // Sleep for 1 millisecond
        usleep(1000);
        refresh_game_screen(xMax, yMax);
    }

    for (int j = 0; j < 5; j++) {
        // Close the read end of the pipe
        close(pipefds[j][0]);
        // Close the write end of the pipe
        close(pipefds[j][1]);
    }
    // Close the ncurses window
    endwin();

    // Wait for all processes to finish
    waitpid(keyboard_manager, NULL, 0);
    waitpid(obstacles, NULL, 0);
    return EXIT_SUCCESS;
}