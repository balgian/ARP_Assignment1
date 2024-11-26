#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>

int main (int argc, char *argv[]) {
  int xMax, yMax;
  char info[23];

  while(1){
  	initscr();
   	nodelay(stdscr, TRUE);
  	while(1){
    	if (read(STDIN_FILENO, &info, 1) == -1){
      		perror("read");
      		endwin();
      		return EXIT_FAILURE;
    	}
    	if (sscanf(info, "o%d,%d", &xMax, &yMax) == 2)
     	 break;
    	printw("%c%c%c", info[0],info[1],info[2]);
    	usleep(10000); // Sleep for 0.01 seconds
  	}


    printw("os");
  	for (int y = 1; y < yMax-1; y++) {
    	for (int x = 1; x < xMax-5; x++) {
        	if (rand() % 100 < 15)
         	 printw("o%d,%d", x, y);
    	}
  	}
    printw("oe");
  	endwin();
  }
  return EXIT_SUCCESS;
}