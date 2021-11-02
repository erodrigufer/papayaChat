#include <ncurses.h> /* required to create terminal UI */
#include <string.h>	/* required for string manipulation */

#include "basics.h" /* includes library to handle errors */
#include "inet_sockets.h" /* include libraray to handle TCP sockets */

/* Define TCP/IP services */
#define SERVICE "51000" /* Port to connect to with server */
#define HOST "kah" /* server address (found, e.g. on /etc/hosts file) */
#define BUF_SIZE 4096 /* bytes transmission size */

/* establish TCP connection with server, return fd of server socket */
static int establishConnection(void){
	int server_fd; /* fd for server connection */

	/* SOCK_STREAM for TCP connection */
	server_fd = clientConnect(HOST,SERVICE,SOCK_STREAM);
	if(server_fd == -1)
		errExit("clientConnect"); /* connection failed, exit */

	return server_fd;
}

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
		errExit("newwin [chatWindow]");

	keypad(chatWindow, TRUE); /* Enable keypad and F-keys */

	/* stdin reading should be non-blocking, if no character is read from
	stdin, then getch returns ERR 
	Otherwise, if it weren't non-blocking, we would not be able to 
	print in the upper half of the chat window concurrently */
	if(nodelay(chatWindow,TRUE)==ERR)
		errExit("nodelay[chatWindow]");		/* nodelay() failed, catastrophic error */

	return chatWindow;
}

/* Get the standard greetings message from the pipe */
static void
getGreetingsMessage(int pipe_fd, char *string_buf)
{
	ssize_t bytesRead;
//	char string_buf[BUF_SIZE];

	while((bytesRead = read(pipe_fd, string_buf, BUF_SIZE)) > 0){
	}
	/* reading from pipe failed */
	if(bytesRead == -1)
		errExit("read greetings message");

}

/* read from server socket and pass data through pipe to frontEnd parent process */
static void
handleReadSocket(int server_fd, int pipe_fd){

	ssize_t bytesRead;
	char string_buf[BUF_SIZE];

	/* endless for-loop reading from the TCP-socket
	every time a whole message is read, then read() returns 0
	and the while-loop will then re-start with a blocking read()
	until some bytes can be read from the socket */
	for(;;){
		while((bytesRead = read(server_fd, string_buf, BUF_SIZE)) > 0){
			if(write(pipe_fd, string_buf, bytesRead) != bytesRead){
				/* the amount of bytes written is not equal to the amount of bytes read */
				errExit("write to pipe [handleReadSocket]");
			}
		}
		/* read() failed, exit programm with error */
		if(bytesRead == -1)
			errExit("read from server");
	}
}

int main(int argc, char *argv[])
{
	/* establish connection with server, get fd to be shared with child processes */
	int server_fd;
	server_fd = establishConnection();

	/* Create child processes which will handle the communication with the server
	1. Child process will send messages to the server
	2. Child process will receive messages from the server
	Both child process will communicate with the parent process handling the front-end through pipes */
	/* Create pipe for 1. Child process, which will send messages to the server.
	The pipe is created before fork() so that the fds are shared between both the
	parent and the child process */
	int pipe_fds_send_server[2];
	if(pipe(pipe_fds_send_server)==-1)
		errExit("pipe send to server");

	/* 1. Child creation -- handles receiving data from server */
	switch(fork()) {
		/* error on fork() call  */
		case -1:
			/* exit with an error message */
			errExit("fork(), 1. Child [send] could not be created.");
			break;

		/* 1. Child process (returns 0) */
		case 0: 
			/* Close stdin of pipe shared with parent */
			close(pipe_fds_send_server[1]);
			/*TODO: add actions of 1. Child process 
			temporarily just make child exit*/
			_exit(EXIT_SUCCESS);
			break;

		/* Parent: fork() returns PID of newly created child */
		default:
			/* Close stdout of pipe shared with 1. Child */
			close(pipe_fds_send_server[0]);
			break;
	}// end switch-case fork 1

	/* 2. Child creation -- handles receiving data from server */
	int pipe_fds_receive_server[2];
	if(pipe(pipe_fds_receive_server)==-1)
		errExit("pipe receive from server");
	switch(fork()) {
		/* error on fork() call  */
		case -1:
			/* exit with an error message */
			errExit("fork(), 2. Child [receive] could not be created.");
			break;

		/* 2. Child process (returns 0) */
		case 0: 
			/* Close stdout of pipe shared with parent */
			close(pipe_fds_receive_server[0]);
			/* Close pipe from 1. Child inherited through parent */
			close(pipe_fds_send_server[1]);
			handleReadSocket(server_fd, pipe_fds_receive_server[1]);
			break;

		/* Parent: fork() returns PID of newly created child */
		default:
			/* Close stdin of pipe shared with 2. Child */
			close(pipe_fds_receive_server[1]);
			break;
	}// end switch-case fork 2

	/* initialize and configure ncurses */
	configureNcurses();

	/* Print 'papayaChat' in the first 0, right in the middle of the screen 
	subtract half of the length of 'papayaChat' from the x position in the
	middle of the screen */
	mvprintw(0,COLS/2-strlen("papayaChat")/2,"papayaChat\n");

	if(hline('_',COLS)==ERR)
		errExit("hline1");
	
	char string_buf[BUF_SIZE];
	getGreetingsMessage(pipe_fds_receive_server,string_buf);
	printw("%s",srting_buf);

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
		errExit("hline2");
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
				/* copy 200 characters at position, into message char array,
				the coordinates to start copying strings, should be the relative
				coordinates inside chatWindow, so 0,0 actually equals 
				y_start,x_start */
				int errorString = mvwinnstr(chatWindow,0,0,message,200);
				if(errorString == ERR){
					endwin();
					errExit("mvwinnstr [chatWindow]");
					}
				/* TODO: after storing contents of line, delete line and pipe
				the contents to the process dealing with sending the messages
				to the back-end server */
				break;
			}

			/* BACKSPACE was pressed, delete characters */
			if(a == KEY_BACKSPACE || a == KEY_LEFT){
				/* get current x position (relative to chatWindow coordinates, so
				x=0, actually equals to x_start) */
				x_position=getcurx(chatWindow);
				/* if cursor position is x=0, then do not do anything 
				(edge case). the coordinates are relative to chatWindow */
				if(x_position==0)
					continue; /* do not continue deleting characters, since the 
								cursor is at the border of the window */
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
