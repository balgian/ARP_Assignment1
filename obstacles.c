#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>

int main (int argc, char *argv[]) {
  int xMax, yMax;
  char info[23];

  initscr();
  nodelay(stdscr, TRUE);
  while(1){
  	while(1){
    	if (read(STDIN_FILENO, &info, sizeof(info)) == -1){
      		perror("read");
      		endwin();
      		return EXIT_FAILURE;
    	}
    	if (sscanf(info, "o%d,%d", &xMax, &yMax) == 2)
          break;
        write(STDOUT_FILENO, info, sizeof(info));
    	usleep(1000); // Sleep for 1 millisecond
  	}

  	snprintf(info, sizeof(info), "os");
  	write(STDOUT_FILENO, info, sizeof(info));
  	snprintf(info, sizeof(info), "o%d,%d", 10, 10);
  	write(STDOUT_FILENO, info, sizeof(info));
//  	for (int y = 1; y < yMax-1; y++) {
//    	for (int x = 1; x < xMax-5; x++) {
//        	if (rand() % 100 < 50) {
//                snprintf(info, sizeof(info), "o%d,%d", x, y);
//                write(STDOUT_FILENO, info, sizeof(info));
//                usleep(100);
//           	}
//    	}
//  	}
  	snprintf(info, sizeof(info), "oe");
  	write(STDOUT_FILENO, info, sizeof(info));
  }
  endwin();
  return EXIT_SUCCESS;
}