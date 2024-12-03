#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

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

    // Make a pipe to send/receive data to/from the child processes
    int pipefds[5][2];
    // Create 5 pipes
    for (int i = 0; i < 5; i++) {
        if (pipe(pipefds[i]) == -1) {
            // Print an error message if pipe creation failed
            perror("Pipe");
            // Exit the program with a failure status
            return EXIT_FAILURE;
        }
    }
    // Create 5 child processes
    pid_t pid[5];
    for (int i = 0; i < 4; i++) {
        pid[i] = fork();
        if (pid[i] == -1) {
            perror("Fork");
            return EXIT_FAILURE;
        }
        if (pid[i] == 0) {
            char read_pipe_str[10];
            char write_pipe_str[10];

            sprintf(read_pipe_str, "%d", pipefds[i][0]);
            sprintf(write_pipe_str, "%d", pipefds[i][1]);

            if (i == 0) {
                execl("./keyboard_manager", "keyboard_manager", read_pipe_str, write_pipe_str, NULL);
                perror("Keyboard Manager");
            }
            if (i == 1) {
                execl("./obstacles", "obstacles", read_pipe_str, write_pipe_str, NULL);
                perror("Keyboard Manager");
            }
            if (i == 2) {
                execl("./targets_generator", "targets_generator", read_pipe_str, write_pipe_str, NULL);
                perror("Keyboard Manager");
            }
            if (i == 3) {
                execl("./drone_dynamics", "drone_dynamics", read_pipe_str, write_pipe_str, NULL);
                perror("Keyboard Manager");
            }
            return EXIT_FAILURE;
        }
    }
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
    char pushChar = '\0';
    while (pushChar != 's') {
        if (read(pipefds[0][0], &pushChar, sizeof(pushChar)) == -1) {
            perror("read");
            close(pipefds[0][0]);
            close(pipefds[0][1]);
            endwin();
            return EXIT_FAILURE;
        }
        if (pushChar == 'q') {
            // TODO: implement the watchdog signlal
            // Write to the watchdog
            // write(pipefds[5][1], &pushChar, sizeof(pushChar));
            endwin();
            return EXIT_SUCCESS;
        }
        refresh();
        usleep(1000);
    }
    refresh_game_screen(xMax, yMax);
    // Refresh the ncurses window to display the changes
    refresh();
    usleep(10000);

    // Send the size of the window to the obstacles process
    char info[22];
    snprintf(info, sizeof(info), "%d,%d", xMax, yMax);
    if (write(pipefds[1][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[1][0]);
        close(pipefds[1][1]);
        endwin();
        return EXIT_FAILURE;
    }
    memset(info, '\0', sizeof(info));
    usleep(1000);

    // Read the obstacles positions and send it to the target generator process
    do {
        if (read(pipefds[1][0], &info, sizeof(info)) == -1) {
            perror("read");
            close(pipefds[1][0]);
            close(pipefds[1][1]);
            endwin();
            return EXIT_FAILURE;
        }
    } while (strcmp(info, "s") != 0);

    int x = 0;
    int y = 0;
    do {
        if (read(pipefds[1][0], &info, sizeof(info)) == -1) {
            perror("read");
            close(pipefds[1][0]);
            close(pipefds[1][1]);
            endwin();
            return EXIT_FAILURE;
        }
        if (sscanf(info, "%d,%d", &x, &y) == 2) {
            if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
                perror("write");
                close(pipefds[2][0]);
                close(pipefds[2][1]);
                endwin();
                return EXIT_FAILURE;
            }
            mvaddch(y, x, 'o');
            refresh();
        }
    } while (strcmp(info, "e") != 0);

    // Send the end message to the target generator process
    if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[2][0]);
        close(pipefds[2][1]);
        endwin();
        return EXIT_FAILURE;
    }
    // Send the size of the window to the target generator process
    snprintf(info, sizeof(info), "%d,%d", xMax, yMax);
    if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[2][0]);
        close(pipefds[2][1]);
        endwin();
        return EXIT_FAILURE;
    }
    memset(info, '\0', sizeof(info));
    usleep(1000);

    // Read the targets positions and send it to the drone dynamics process
    int num_target = 0;
    do {
        if (read(pipefds[2][0], &info, sizeof(info)) == -1) {
            perror("read");
            close(pipefds[2][0]);
            close(pipefds[2][1]);
            endwin();
            return EXIT_FAILURE;
        }
        if (sscanf(info, "%d,%d", &x, &y) == 2) {
            mvprintw(y, x, "%d", num_target++);
            refresh();
        }
    } while (strcmp(info, "e") != 0);

    mvprintw(0, xMax - 23, " Press p to pause ");
    refresh();

    x = (xMax-5)/2;
    y = (yMax - 1)/2;
    while (mvinch(y, x) != ' '){
        x = x + (random() % 2 == 0 ? 1 : -1);
        y = y + (random() % 2 == 0 ? 1 : -1);
    }
    mvaddch(y, x, '+');
    refresh();

    // Sets the keyboard manager's pipe to non-blocking mode
    int flags = fcntl(pipefds[0][0], F_GETFL);
    if (flags == -1) {
        perror("fcntl");
        close(pipefds[0][0]);
        close(pipefds[0][1]);
        endwin();
        return EXIT_FAILURE;
    }
    if (fcntl(pipefds[0][0], F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        close(pipefds[0][0]);
        close(pipefds[0][1]);
        endwin();
        return EXIT_FAILURE;
    }
    int x_drone_positions[3] = { x, x, x };
    int y_drone_positions[3] = { y, y, y };
    int x_shifts = 0;
    int y_shifts = 0;

    char c = '\0';
    bool game_pause = false;
    do {
        // Receive the data from the keyboard manager
        if (read(pipefds[0][0], &c, sizeof(c)) == -1) {
            int x_variation = 0;
            int y_variation = 0;
            // Vertical movements are on the y axis and horizontal movements are on the x axis
            switch (c) {
                case 'w': // Up Left
                    x_variation--;
                    y_variation++;
                    break;
                case 'e': // Up
                    y_variation++;
                    break;
                case 'r': // Up Right or Reset
                    // TODO: Reset the game
                    x_variation++;
                    y_variation++;
                    break;
                case 's': // Left or Suspend
                    // TODO: Suspend the game
                    x_variation--;
                    break;
                case 'd': // Breake
                    x_variation = 0;
                    y_variation = 0;
                    break;
                case 'f': // Right
                    x_variation++;
                    break;
                case 'x': // Down Left
                    x_variation--;
                    y_variation--;
                    break;
                case 'c': // Down
                    y_variation--;
                    break;
                case 'v': // Down Right
                    x_variation++;
                    y_variation--;
                    break;
                case 'p': // Pause
                    game_pause = !game_pause;
                    break;
                case 'q': // Quit
                    // TODO: Send the end message to the watchdog process
                    break;
                default:
                    break;
            }
            char info[64];
            snprintf(info, sizeof(info), "%d,%d,%d,%d,%d,%d,%d,%d", x_drone_positions[0], x_drone_positions[1],
                x_drone_positions[2], y_drone_positions[0], y_drone_positions[1], y_drone_positions[2],
                x_variation, y_variation);
            if (write(pipefds[3][1], &info, sizeof(info)) == -1) {
                perror("write");
                close(pipefds[3][0]);
                close(pipefds[3][1]);
                endwin();
                return EXIT_FAILURE;
            }
            char schifts[22];

            if (read(pipefds[1][0], &schifts, sizeof(schifts)) == -1) {
                perror("read");
                close(pipefds[1][0]);
                close(pipefds[1][1]);
                endwin();
                return EXIT_FAILURE;
            }
            sscanf(schifts, "%d,%d", &x_shifts, &y_shifts);
        }

        int x_abs = abs(x_shifts);
        int y_abs = abs(y_shifts);
        int x_sign = x_shifts >= 0 ? 1 : -1;
        int y_sign = y_shifts >= 0 ? 1 : -1;

        int max = x_abs > y_abs ? x_abs : y_abs;

        for (int i = 0; i < max; i++) {
            mvaddch(y, x, ' ');
            y = y + (y_abs != 0 ? y_sign : 0);
            x = x + (x_abs != 0 ? x_sign : 0);
            mvaddch(y, x, '+');
            refresh();
            usleep(100300);
        }

        x_drone_positions[0] = x_drone_positions[1];
        x_drone_positions[1] = x_drone_positions[2];
        x_drone_positions[2] = x;
        y_drone_positions[0] = y_drone_positions[1];
        y_drone_positions[1] = y_drone_positions[2];
        y_drone_positions[2] = y;
    } while (c != 'q' || game_pause == false);

    for (int i = 0; i < 5; i++) {
        // Close the read end of the pipe
        close(pipefds[i][0]);
        // Close the write end of the pipe
        close(pipefds[i][1]);
    }
    // Close the ncurses window
    endwin();

    // TODO: implement the watchdog signlal
    // Write to the watchdog
    // write(pipefds[5][1], &pushChar, sizeof(pushChar));

    // Wait for the child processes to finish
    for (int i = 0; i < 5; i++) {
        waitpid(pid[i], NULL, 0);
    }
    return EXIT_SUCCESS;
}