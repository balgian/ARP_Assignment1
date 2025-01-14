#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main (int argc, char *argv[]) {
  // TODO: fprintf doesn't work here
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <read_fd> <write_fd>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int read_fd = atoi(argv[1]);
  int write_fd = atoi(argv[2]);

  int xMax, yMax;
  char info[22];

  while(1){
    if (read(read_fd, &info, sizeof(info)) == -1){
      perror("read");
      return EXIT_FAILURE;
    }
    sscanf(info, "%d,%d", &xMax, &yMax);
    snprintf(info, sizeof(info), "s");
    write(write_fd, &info, sizeof(info));
  	for (int y = 1; y < yMax-1; y++) {
    	for (int x = 1; x < xMax-5; x++) {
        	if (rand() % 100 < 0.0005) {
                snprintf(info, sizeof(info), "%d,%d", x, y);
                if (write(write_fd, &info, sizeof(info)) == -1) {
                    perror("write");
                    return EXIT_FAILURE;
                }
           	}
    	}
  	}
	snprintf(info, sizeof(info), "e");
  	write(write_fd, &info, sizeof(info));
  }
  return EXIT_SUCCESS;
}