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

  int  xMax, yMax;
  if (read(read_fd, &info, sizeof(info)) == -1) {
    perror("read");
    close(read_fd);
    close(write_fd);
    return EXIT_FAILURE;
  }
  sscanf(info, "%d,%d", &xMax, &yMax);

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
    char info_drone[64];
    if (read(read_fd, &info_drone, sizeof(info_drone)) == -1) {
      perror("read");
      return EXIT_FAILURE;
    }
    sscanf(info, "%d,%d,%d,%d,%d,%d,%d,%d", &x[0], &x[1], &x[2], &y[0], &y[1], &y[2], &x_variation, &y_variation);
    // Implementing the equation
    double M = 1.0;      // Mass
    double K = 1.0;      // Viscous coefficient
    double T = 55.0;      // Time interval

    double sumFx = (M * (x[0] + x[2] - 2 * x[1]) / (T * T)) + (K * (x[2] - x[1]) / T);
    double sumFy = (M * (y[0] + y[2] - 2 * y[1]) / (T * T)) + (K * (y[2] - y[1]) / T);

    // Calculate the acceleration
    double ax = sumFx / M;
    double ay = sumFy / M;

    // Calculate the velocity change
    double vx = x_variation * ax * T;
    double vy = y_variation * ay * T;

    // Calculate the position change
    double x_shifts_normalized = vx * T;
    double y_shifts_normalized = vy * T;

    int x_shifts = (int)x_shifts_normalized;
    int y_shifts = (int)y_shifts_normalized;

    char schifts[64];
    sprintf(schifts, "%d,%d", x_shifts, y_shifts);
    if (write(write_fd, &schifts, strlen(schifts)) == -1) {
      perror("write");
      return EXIT_FAILURE;
    }

    usleep(1000);
  }
  return 0;
}