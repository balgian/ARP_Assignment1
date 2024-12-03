#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <read_fd> <write_fd>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int read_fd = atoi(argv[1]);
  int write_fd = atoi(argv[2]);

  int x[3], y[3], x_variation, y_variation;
  char info[64];
  if (read(read_fd, &info, sizeof(info)) == -1) {
    perror("read");
    return EXIT_FAILURE;
  }
  sscanf(info, "%d,%d,%d,%d,%d,%d,%d,%d",
         &x[0], &x[1], &x[2], &y[0], &y[1], &y[2], &x_variation, &y_variation);

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
  int x_shifts = (int)(vx * T);
  int y_shifts = (int)(vy * T);

  char schifts[22];
  sprintf(schifts, "%d,%d", x_shifts, y_shifts);
  if (write(write_fd, &schifts, sizeof(schifts)) == -1) {
    perror("write");
    return EXIT_FAILURE;
  }
  return 0;
}