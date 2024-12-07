#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h>

void refresh_game_screen(const int xMax, const int yMax) {
    // Clear the inside of the box
    for (int y = 1; y < yMax-1; y++) {
        for (int x = 1; x < xMax-5; x++) {
            mvaddch(y, x, ' ');
        }
    }
    refresh();
}

int main(void) {

  	// * Make the pipes with child processes, and if an error occurs:
        // * - Print an error message if pipe creation failed;
        // * - Exit the program with a failure status.
    int pipefds[5][2];
    for (int i = 0; i < 5; i++) {
        if (pipe(pipefds[i]) == -1) {
            perror("Pipe");
            return EXIT_FAILURE;
        }
    }

  	// * Fork five child processes.
    pid_t pid[5];
    for (int i = 0; i < 5; i++) {
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

            if (i == 0) execl("./keyboard_manager", "keyboard_manager", read_pipe_str, write_pipe_str, NULL);
            if (i == 1) execl("./obstacles", "obstacles", read_pipe_str, write_pipe_str, NULL);
            if (i == 2) execl("./targets_generator", "targets_generator", read_pipe_str, write_pipe_str, NULL);
            if (i == 3) execl("./drone_dynamics", "drone_dynamics", read_pipe_str, write_pipe_str, NULL);
            if (i == 4) execl("./watchdog", "watchdog", read_pipe_str, write_pipe_str, NULL);

            perror("execl");
            return EXIT_FAILURE;
        }
    }

    // * Setting up `select`.
    fd_set readfds;
    int max_fd = 0;
    for (int i = 0; i < 5; i++) {
        if (pipefds[i][0] > max_fd) {
            max_fd = pipefds[i][0];
        }
    }

  	// * Initialization and setup of the map:
    	// * - Open a new ncurses window;
        // * - Deactivate mouse support;
        // * - Delete the echo of the keys;
        // * - Delete the cursor highlight.
    initscr();
    mousemask(0, NULL);
    noecho();
    curs_set(0);

    // * Create a border around the whole window:
    	// * - Get the maximum x and y coordinates of the window;
    	// * - Print the top border of the window:
    		// * - Place the string '-' at the top of the window at the current x coordinate;
    		// * - Place the string '-' at the bottom of the window at the current x coordinate.
    	// * - Print the sides of the window:
    		// * - Place the string '|' at the left of the window at the current y coordinate;
    		// * - Place the string '|' at the right of the window at the current y coordinate.
    	// * - Print the corners of the window:
    		// * - Top left;
    		// * - Top right;
    		// * - Bottom left;
    		// * - Bottom right.
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    for (int i = 0; i < xMax-5; i++) {
        mvaddch(0, i, '-');
        mvaddch(yMax-1, i, '-');
    }
    for (int i = 1; i < yMax-1; i++) {
        mvaddch(i, 0, '|');
        mvaddch(i, xMax-5, '|');
    }
    mvaddch(0, 0, '+');
    mvaddch(0, xMax-5, '+');
    mvaddch(yMax-1, 0, '+');
    mvaddch(yMax-1, xMax-5, '+');

    // * Write the strings "Push s to start" and
    // * "Press q to quit" in the middle of the window
    int middleX = (xMax-5)/2;
    int middleY = (yMax - 1)/2;
    mvprintw(middleY, middleX, "Press s to start");
    mvprintw(middleY + 1, middleX, "Press q to quit");
    refresh();

    // * Wait for the user to press the key 's'
    char pushChar = '\0';
    while (pushChar != 's') {
      	FD_ZERO(&readfds);
      	FD_SET(pipefds[0][0], &readfds);

   		if (select(pipefds[0][0] + 1, &readfds, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(pipefds[0][0], &readfds)) {
                if (read(pipefds[0][0], &pushChar, sizeof(pushChar)) == -1) {
                    perror("read");
                    close(pipefds[0][0]);
                    close(pipefds[0][1]);
                    endwin();
                    return EXIT_FAILURE;
                }
            }
        }
        if (pushChar == 'q') {
            // TODO: implement the watchdog signlal
            endwin();
            return EXIT_SUCCESS;
        }
    }
    refresh_game_screen(xMax, yMax);

    // * Send the size of the window to the child processes:
    // * - 'obstacles';
    // * - 'targets_generator';
    // * - 'drone dynamics'
    char info[22];
    snprintf(info, sizeof(info), "%d,%d", xMax, yMax);
    if (write(pipefds[1][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[1][0]);
        close(pipefds[1][1]);
        endwin();
        return EXIT_FAILURE;
    }
    if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[2][0]);
        close(pipefds[2][1]);
        endwin();
        return EXIT_FAILURE;
    }
    if (write(pipefds[3][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[3][0]);
        close(pipefds[3][1]);
        endwin();
        return EXIT_FAILURE;
    }

    int x = 0;
    int y = 0;
    // * Insert the obstacoles in the map and send their position to the child
    // * processes: 'targets_generator' and 'drone dynamics'
    do {
        FD_ZERO(&readfds);
        FD_SET(pipefds[1][0], &readfds);
        if (select(pipefds[1][0] + 1, &readfds, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(pipefds[1][0], &readfds)) {
                if (read(pipefds[1][0], &info, sizeof(info)) == -1) {
                    perror("read");
                    close(pipefds[1][0]);
                    close(pipefds[1][1]);
                    endwin();
                    return EXIT_FAILURE;
                }
            }
        }

        if (sscanf(info, "%d,%d", &x, &y) == 2) {
            if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
                perror("write");
                close(pipefds[2][0]);
                close(pipefds[2][1]);
                endwin();
                return EXIT_FAILURE;
            }
            if (write(pipefds[3][1], &info, sizeof(info)) == -1) {
                perror("write");
                close(pipefds[3][0]);
                close(pipefds[3][1]);
                endwin();
                return EXIT_FAILURE;
            }
            mvaddch(y, x, 'o');
            refresh();
        }
    } while (strcmp(info, "e") != 0);
    refresh();

    // * Send the end message to the: 'target generator' and
    // * 'drone dynamic' child processes
    if (write(pipefds[2][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[2][0]);
        close(pipefds[2][1]);
        endwin();
        return EXIT_FAILURE;
    }
    if (write(pipefds[3][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[3][0]);
        close(pipefds[3][1]);
        endwin();
        return EXIT_FAILURE;
    }

    // * Insert the targets and send their positon to the child
    // * process 'drone dynamics' process
    int num_target = 0;
    do {

      FD_ZERO(&readfds);
      FD_SET(pipefds[2][0], &readfds);
      if (select(pipefds[2][0] + 1, &readfds, NULL, NULL, NULL) > 0) {
        if (FD_ISSET(pipefds[2][0], &readfds)) {
          if (read(pipefds[2][0], &info, sizeof(info)) == -1) {
            perror("read");
            close(pipefds[2][0]);
            close(pipefds[2][1]);
            endwin();
            return EXIT_FAILURE;
          }
        }
      }

      if (sscanf(info, "%d,%d", &x, &y) == 2) {
        if (write(pipefds[3][1], &info, sizeof(info)) == -1) {
          perror("write");
          close(pipefds[3][0]);
          close(pipefds[3][1]);
          endwin();
          return EXIT_FAILURE;
        }
        mvprintw(y, x, "%d", num_target++);
      }
    } while (strcmp(info, "e") != 0);
    refresh();

    // * Send the end message to the 'drone dynamic' child process
    if (write(pipefds[3][1], &info, sizeof(info)) == -1) {
        perror("write");
        close(pipefds[3][0]);
        close(pipefds[3][1]);
        endwin();
        return EXIT_FAILURE;
    }

    // * Print "Press p to pause" in the top right corner of the screen
    mvprintw(0, xMax - 23, " Press p to pause ");
    refresh();

    x = (xMax-5)/2;
    y = (yMax - 1)/2;
    int max_iterations = 1000;
    int iterations = 0;
    while (mvinch(y, x) != ' ' && iterations < max_iterations){
        x = x + (random() % 2 == 0 ? 1 : -1);
        y = y + (random() % 2 == 0 ? 1 : -1);
        iterations++;
    }
    if (iterations == max_iterations) {
        x = (xMax-5)/2;
        y = (yMax - 1)/2;
    }
    mvaddch(y, x, '+');
    refresh();

    /*
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

    int drone_positions[6] = { x, y, x, y, x, y };
    int variance[2] = { 0, 0 };

    char c;
    bool game_pause = false;
    do {
        if (read(pipefds[0][0], &c, sizeof(c)) > 0) {
            switch (c) {
                case 'w': // Up Left
                    variance[1]--;
                    variance[0]--;
                    break;
                case 'e': // Up
                    variance[1]--;
                	break;
                case 'r': // Up Right or Reset
                    variance[1]--;
                    variance[0]++;
                    break;
                case 's': // Left or Suspend
                    variance[0]--;
                	break;
                case 'd': // Breake
                  	variance[0] = 0;
                   	variance[1] = 0;
                    break;
                case 'f': // Right
                    variance[0]++;
                	break;
                case 'x': // Down Left
                    variance[1]++;
                	variance[0]--;
                	break;
                case 'c': // Down
                    variance[1]++;
                	break;
                case 'v': // Down Right
                    variance[1]++;
                	variance[0]++;
                	break;
                case 'p': // Pause
                    game_pause = !game_pause;
                	break;
                case 'q': // Quit
                    break;
                default:
                    break;
            }
        }

        char info_drone[64];
        sprintf(info_drone, "%d,%d,%d,%d,%d,%d,%d,%d", drone_positions[0], drone_positions[1], drone_positions[2],
            drone_positions[3], drone_positions[4], drone_positions[5], variance[0], variance[1]);

        if (write(pipefds[3][1], &info_drone, sizeof(info_drone)) == -1){
            perror("write");
            close(pipefds[3][0]);
            close(pipefds[3][1]);
            endwin();
            return EXIT_FAILURE;
        }
        memset(info_drone, '\0', sizeof(info));
        usleep(100);

        mvprintw(3, xMax - 23, " %s", info_drone);
        refresh();
        char shifts_drone[22];
        int shifts[2] = { 0, 0 };
        if (read(pipefds[3][0], &shifts_drone, sizeof(shifts_drone)) == -1) {
            perror("read");
            close(pipefds[3][0]);
            close(pipefds[3][1]);
            endwin();
            return EXIT_FAILURE;
        }
        int x_tmp, y_tmp;
        if (sscanf(shifts_drone, "%d,%d", &x_tmp, &y_tmp) == 2) {
            shifts[0] = x_tmp;
            shifts[1] = y_tmp;
        }

        int shifts_sign[] = {shifts[0] >= 0 ? 1 : -1, shifts[1] >= 0 ? 1 : -1};
        int max = abs(shifts[0]) > abs(shifts[1]) ? abs(shifts[0]) : abs(shifts[1]);

        for (int i = 0; i < max; i++) {
            mvaddch(y, x, ' ');
            x = x + (shifts[0] != 0 ? shifts_sign[0] : 0);
            y = y + (shifts[1] != 0 ? shifts_sign[1] : 0);
            mvaddch(y, x, '+');
            refresh();
            usleep(30000);
        }

        drone_positions[0] = drone_positions[2];
        drone_positions[2] = drone_positions[4];
        drone_positions[4] = x;
        drone_positions[1] = drone_positions[3];
        drone_positions[3] = drone_positions[5];
        drone_positions[5] = y;
    } while (c != 'q' || game_pause == false);
     */
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