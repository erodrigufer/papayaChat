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
	mvprintw(0,COLS/2-strlen("papayaChat")/2,"papayaChat\n\n");
	/* after this print statement, the cursor can be found two lines below the title */
	refresh();

	//	WINDOW * input_window;

	//input_window = newwin(LINES/2,COLS,LINES/2,0);

	// mvwprintw(input_window,10,10,"Input");
	//wrefresh(input_window);

	//wrefresh(stdscr);	/* Refresh real screen */

	/* stdin reading should be non-blocking, if no character is read from
	stdin, then getch returns ERR */
	if(nodelay(stdscr,TRUE)==ERR)
		exit(EXIT_FAILURE);		/* nodelay() failed, catastrophic error */

	int a = 0;			/* TODO: uninitialized ERROR found here fix that*/


	int x_position = 0;
	int y_position = 0;

	/* string for message */
	char message[200];

	/* Draw horizontal line */
	if(hline('_',COLS)==ERR)
		exit(EXIT_FAILURE);

	/* starting position for messages */
	int x_start = 0;
	int y_start = 25;
	move(y_start,x_start); /* move cursor to start position */


	/* Stop ncurses when 'q' is pressed */
	while(a!= KEY_DOWN){
		a = getch();
		/* if getch returns ERR then no character was typed,
		getch was configured as non-blocking with the nodelay() function */
		if(a != ERR){
			/* carriage return was pressed, go to next line and move cursor */
			if(a == '\n'){
				/* move to start position */
				mvinnstr(y_start,x_start,message,200);
				break;
			}

			/* TODO: if x-position == 0, edge case, should do nothing */
			/* BACKSPACE was pressed, delete characters */
			if(a == KEY_BACKSPACE || a == KEY_LEFT){
				/* get current x position */
				x_position=getcurx(stdscr);
				/* move cursor to the left to delete last pressed character */
				x_position--;
				y_position=getcury(stdscr);
				move(y_position,x_position); /* move cursor to new position */
				/* delete character under cursor */
				delch();
				continue;
			}
			/* a character was read, print and move cursor */
			addch(a);

			refresh();
		}
    }

	endwin();			/* End ncurses */

	printf("%s\n",message);
	exit(EXIT_SUCCESS);
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
