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
	mvprintw(0,COLS/2-strlen("papayaChat")/2,"papayaChat\n");
	
	if(hline('_',COLS)==ERR)
		exit(EXIT_FAILURE);

	refresh();

	int a = 0;			/* TODO: uninitialized ERROR found here fix that*/


	int x_position = 0;
	int y_position = 0;

	/* string for message */
	char message[200];

	/* starting position for messages */
	int x_start = 1;
	int y_start = (int) LINES*3/4;
	move(y_start-1,0); /* move cursor to start position */

	/* Draw horizontal line */
	if(hline('_',COLS)==ERR)
		exit(EXIT_FAILURE);
	/* refresh() is needed to depict horizontal line */
	refresh();

	/* create separate window for chat input */
	WINDOW * chatWindow;
	chatWindow = newwin(LINES-y_start,COLS-1,y_start,x_start);
	if(chatWindow == NULL)
		exit(EXIT_FAILURE);
	keypad(chatWindow, TRUE); /* Enable keypad and F-keys */

	/* stdin reading should be non-blocking, if no character is read from
	stdin, then getch returns ERR */
	if(nodelay(chatWindow,TRUE)==ERR)
		exit(EXIT_FAILURE);		/* nodelay() failed, catastrophic error */

	/* Stop ncurses when 'KEY_DOWN' is pressed */
	while(a!= KEY_DOWN){
		a = wgetch(chatWindow);
		/* if getch returns ERR then no character was typed,
		getch was configured as non-blocking with the nodelay() function */
		if(a != ERR){
			/* carriage return was pressed, go to next line and move cursor */
			if(a == '\n'){
				/* copy 200 characters at position, into message char array */
				int errorString = mvwinnstr(chatWindow,y_start,x_start,message,200);
				if(errorString == ERR){
					endwin();
					exit(EXIT_FAILURE);
					}
				/* TODO: after storing contents of line, delete line and pipe
				the contents to the process dealing with sending the messages
				to the back-end server */
				break;
			}

			/* TODO: if x-position == 0, edge case, should do nothing */
			/* BACKSPACE was pressed, delete characters */
			if(a == KEY_BACKSPACE || a == KEY_LEFT){
				/* get current x position */
				x_position=getcurx(chatWindow);
				/* move cursor to the left to delete last pressed character */
				x_position--;
				y_position=getcury(chatWindow);
				wmove(chatWindow,y_position,x_position); /* move cursor to new position */
				/* delete character under cursor */
				wdelch(chatWindow);
				continue;
			}
			/* a character was read, print and move cursor */
			waddch(chatWindow,a);

			wrefresh(chatWindow);
		}
    }

	endwin();			/* End ncurses */

	printf("%s\n",message);
	exit(EXIT_SUCCESS);
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
