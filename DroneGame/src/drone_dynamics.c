#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <read_fd> <write_fd>\n", argv[0]);
    return EXIT_FAILURE;
  }
  int read_fd = atoi(argv[1]);
  int write_fd = atoi(argv[2]);

  char info[22];

  // * Receive the size of the window
  int xMax, yMax;
  if (read(read_fd, &info, sizeof(info)) == -1) {
    perror("read");
    close(read_fd);
    close(write_fd);
    return EXIT_FAILURE;
  }
  sscanf(info, "%d,%d", &xMax, &yMax);

  // Receive the obstacles positions
  int x_obst, y_obst;
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

  // Receive the targets positions
  int x_targ, y_targ;
  int *pos_targ = NULL;
  size = 0;
  do {
    if (read(read_fd, &info, sizeof(info)) == -1) {
      perror("read");
      return EXIT_FAILURE;
    }
    if (sscanf(info, "%d,%d", &x_targ, &y_targ) == 2) {
      pos_targ = (int *) realloc(pos_targ, (size+2)*sizeof(int));
      if (!pos_targ) {
        perror("realloc");
        return EXIT_FAILURE;
      }
      pos_targ[size] = x_targ;
      pos_targ[size+1] = y_targ;
      size += 2;
    }
  } while (strcmp(info, "e") != 0);

  while(1) {
    int x[3], y[3], x_variation, y_variation;
    for (int i = 0; i < 3; i++) {
      if (read(read_fd, &info, sizeof(info)) == -1) {
        perror("read");
        return EXIT_FAILURE;
      }
      sscanf(info, "%d,%d", &x[i], &y[i]);
    }
    if (read(read_fd, &info, sizeof(info)) == -1) {
      perror("read");
      return EXIT_FAILURE;
    }
    sscanf(info, "%d,%d", &x_variation, &y_variation);

    // Implementing the equation
    double M = 1.0;      // Mass
    double K = 1.0;      // Viscous coefficient
    double T = 55.0;      // Time interval

    double sumFx = x_variation * ((M * (x[0] + x[2] - 2 * x[1]) / (T * T)) + (K * (x[2] - x[1]) / T));
    double sumFy = y_variation * ((M * (y[0] + y[2] - 2 * y[1]) / (T * T)) + (K * (y[2] - y[1]) / T));

    double accel_x = sumFx / M;
    double accel_y = sumFy / M;

    double velocity_x = accel_x * T;
    double velocity_y = accel_y * T;

    int x_shifts = (int)(velocity_x * T) +1;
    int y_shifts = (int)(velocity_y * T) +1;

    sprintf(info, "%d,%d", x_shifts, y_shifts);
    if (write(write_fd, &info, sizeof(info)) == -1) {
      perror("write");
      return EXIT_FAILURE;
    }
    usleep(1000);
  }
  return 0;
}