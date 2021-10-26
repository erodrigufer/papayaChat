#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int main()
{

	initscr();			/* Start curses mode 		  */
	start_color();		/* Add colour functionality */
	cbreak();			/* No line buffering -> terminal input is
						received immediately, no waiting for carriage
						return (ENTER), CTRL-C and CTRL-Z still work
						though, to send STOP and TERM signals to process */
	keypad(stdscr, TRUE); /* Enable keypad and F-keys */
	noecho();			/* Do not automatically echo characters typed by user */

	/* Print 'papayaChat' in the first 0, right in the middle of the screen 
	subtract half of the length of 'papayaChat' from the x position in the
	middle of the screen */
	mvwprintw(stdscr,0,COLS/2-strlen("papayaChat")/2,"papayaChat\n");

	//	WINDOW * input_window;

	//input_window = newwin(LINES/2,COLS,LINES/2,0);

	// mvwprintw(input_window,10,10,"Input");
	//wrefresh(input_window);
	wprintw(stdscr,"Comm received from server\n");

	wrefresh(stdscr);	/* Refresh real screen */

	/* stdin reading should be non-blocking, if no character is read from
	stdin, then getch returns ERR */
	if(nodelay(stdscr,TRUE)==ERR)
		exit(EXIT_FAILURE);		/* nodelay() failed, catastrophic error */

	char a;
	/* Stop ncurses when 'q' is pressed */
	while(a!='q'){
		a = getch();
    }
	endwin();			/* End curses mode		  */

	exit(EXIT_SUCCESS);
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
