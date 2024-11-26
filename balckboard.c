#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/wait.h>

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
    int pipefd [2];
    // Create a pipe
    if (pipe(pipefd) == -1) {
        // Print an error message if pipe creation failed
        perror("Pipe");
        // Exit the program with a failure status
        return EXIT_FAILURE;
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
        close(pipefd[0]);
        // Duplicate the write end of the pipe to the standard output
        dup2(pipefd[1], STDOUT_FILENO);
        // The child process will execute the program "./keyboard_manager" with the argument "keyboard_manager" and the pipe
        // If the return value is not 0, then there was an error
        execl("./keyboard_manager", "keyboard_manager", (char *)pipefd, NULL);
        // Duplicate the write end of the pipe to the standard output
        close(pipefd[1]);
        // Print an error message if execl failed
        perror("Keyboard manager");
        // Exit the program with a failure status
        return EXIT_FAILURE;
    }

    const pid_t drone_dynamics = fork();
    if (drone_dynamics == -1) {
        perror("Drone dynamics");
        return EXIT_FAILURE;
    }
    if (drone_dynamics == 0) {
        // Duplicate the read end of the pipe to the standard input
        dup2(pipefd[0], STDIN_FILENO);
        // Duplicate the write end of the pipe to the standard output
        dup2(pipefd[1], STDOUT_FILENO);

        execl("./drone_dynamics", "drone_dynamics", NULL);
        close(pipefd[1]);
        perror("Drone dynamics");
        return EXIT_FAILURE;
    }

    const pid_t obstacles = fork();
    if (obstacles == -1) {
        perror("Obstacles");
        return EXIT_FAILURE;
    }
    if (obstacles == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);

        execl("./obstacles", "obstacles", NULL);
        close(pipefd[1]);
        perror("Obstacles");
        return EXIT_FAILURE;
    }

    const pid_t targets_generator = fork();
    if (targets_generator == -1) {
        perror("Targets generator");
        return EXIT_FAILURE;
    }
    if (targets_generator == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);

        execl("./targets_generator", "targets_generator", NULL);
        close(pipefd[1]);
        perror("Targets generator");
        return EXIT_FAILURE;
    }

    const pid_t watchdog = fork();
    if (watchdog == -1) {
        perror("Watchdog");
        return EXIT_FAILURE;
    }
    if (watchdog == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);

        execl("./watchdog", "watchdog", NULL);
        close(pipefd[1]);
        perror("Watchdog");
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
        if (read(pipefd[0], &pushChar, sizeof(pushChar)) == -1) {
            perror("read");
            close(pipefd[0]);
            close(pipefd[1]);
            endwin();
            return EXIT_FAILURE;
        }
        if (pushChar[0] == 'i' && pushChar[1] == 'q') {
            pushChar[0] = 'w';
            pushChar[1] = 'q';
            write(pipefd[1], &pushChar, sizeof(pushChar));
            close(pipefd[0]);
            close(pipefd[1]);
            endwin();
            return EXIT_SUCCESS;
        }
        mvprintw(middleY - 3, middleX, "ci sono %d", i++);
        refresh();
        usleep(10000); // Sleep for 0.01 seconds
    } while (pushChar[0] != 'i' || pushChar[1] != 's');

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
                // Send the quit to the other processes
                char quitChar = 'q';
                write(pipefd[1], &quitChar, sizeof(quitChar));
                break;
            }
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

    // Close the read end of the pipe
    close(pipefd[0]);
    // Close the write end of the pipe
    close(pipefd[1]);
    // Close the ncurses window
    endwin();

    // Wait for all processes to finish
    waitpid(keyboard_manager, NULL, 0);
    waitpid(drone_dynamics, NULL, 0);
    waitpid(obstacles, NULL, 0);
    waitpid(targets_generator, NULL, 0);
    waitpid(watchdog, NULL, 0);
    return 0;
}