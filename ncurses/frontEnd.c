#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

/* function to configure and initialize the ncurses environment */
static void configureNcurses(void)
{

	initscr();			/* Start curses mode 		  */
	start_color();		/* Add colour functionality */
	cbreak();			/* No line buffering -> terminal input is
						received immediately, no waiting for carriage
						return (ENTER), CTRL-C and CTRL-Z still work
						though, to send STOP and TERM signals to process */
	keypad(stdscr, TRUE); /* Enable keypad and F-keys */
	noecho();			/* Do not automatically echo characters typed by user */


}

/* function to configure and create a window for the chat (lower
half of screen)
y_start and x_start are the y,x-coordinates where the window should 
be initialized */
static WINDOW * configureChatWindow(int y_start, int x_start)
{

	/* create separate window for chat input */
	WINDOW * chatWindow;
	/* height is LINES-y_start
	width is COLS -2 */
	chatWindow = newwin(LINES-y_start,COLS-2,y_start,x_start);
	if(chatWindow == NULL)	/* there was an error */
		exit(EXIT_FAILURE);

	keypad(chatWindow, TRUE); /* Enable keypad and F-keys */

	/* stdin reading should be non-blocking, if no character is read from
	stdin, then getch returns ERR 
	Otherwise, if it weren't non-blocking, we would not be able to 
	print in the upper half of the chat window concurrently */
	if(nodelay(chatWindow,TRUE)==ERR)
		exit(EXIT_FAILURE);		/* nodelay() failed, catastrophic error */

	return chatWindow;
}

int main(int argc, char *argv[])
{
	/* initialize and configure ncurses */
	configureNcurses();

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

	WINDOW * chatWindow;
	chatWindow=configureChatWindow(y_start,x_start);

	/* Stop ncurses when 'KEY_DOWN' is pressed */
	while(a!= KEY_DOWN){
		a = wgetch(chatWindow);
		/* if getch returns ERR then no character was typed,
		getch was configured as non-blocking with the nodelay() function */
		if(a != ERR){
			/* carriage return was pressed, go to next line and move cursor */
			if(a == '\n'){
				/* copy 200 characters at position, into message char array */
				/* TODO: mvwinnstr is not working for chatWindow */
				//int errorString = mvwinnstr(chatWindow,y_start,x_start,message,200);
				int errorString = mvinnstr(0,0,message,200); /* strangely this function
				is properly working and with stdscr */
				if(errorString == ERR){
					endwin();
					fprintf(stderr,"mvwinnstr failed.\n");
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
			/* add new read character to chatWindow */
			waddch(chatWindow,a);

			wrefresh(chatWindow);
		}
    }

	endwin();			/* End ncurses */

	printf("%s\n",message);
	exit(EXIT_SUCCESS);
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
