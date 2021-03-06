#include <stdio.h>
#include <fcntl.h>   /* change fds to O_NONBLOCK */
#include <ncurses.h> /* required to create terminal UI */
#include <string.h>	/* required for string manipulation, e.g. strlen() */
#include <signal.h>				/* check 'man 2 sigaction' signal.h is needed
								to change the disposition of signals with
								the sigaction() syscall */	
#include <sys/wait.h>	/* wait on child processes */

#include "basics.h" /* includes library to handle errors */
#include "inet_sockets.h" /* include library to handle TCP sockets */
#include "signalHandling.h" /* functions to handle signals */
#include "handleMessages.h" /* functions to send/receive messages from pipes and sockets */
#include "configParser.h"	/* function to parse config files */

/* Load TCP/IP services from CONFIG.h header */
#include "CONFIG.h" /* file defines IP and port of chat service server */

#define MAX_MESSAGE_SENT (int)COLS*3/4 /* max message size of message to be sent */

/* handle backspace portably, Mac OS presents some problems with recognizing
BACKSPACE
And in other Unix systems, only use KEY_BACKSPACE */
#ifdef __APPLE__
#define PORTABLE_BACKSPACE 127 // recognized as BACKSPACE on Mac OS
#else
#define PORTABLE_BACKSPACE KEY_BACKSPACE
#endif

/* Define colour pair names for ncurses */
#define INSTRUCTIONS_COLOUR 1

/* function to configure and initialize the ncurses environment */
static void 
configureNcurses(void)
{

	initscr();			/* Start curses mode 		  */
	start_color();		/* Add colour functionality */
	/* colour pairs: first colour is text colour, second colour is background
	colour */
	init_pair(INSTRUCTIONS_COLOUR, COLOR_CYAN, COLOR_WHITE);	/* add colour pair */
	cbreak();			/* No line buffering -> terminal input is
						received immediately, no waiting for carriage
						return (ENTER), CTRL-C and CTRL-Z still work
						though, to send STOP and TERM signals to process */
	keypad(stdscr, TRUE); /* Enable keypad and F-keys */
	noecho();			/* Do not automatically echo characters typed by user */


}

/* function to configure and create a window for the chat (upper
half of screen)
y_start and x_start are the y,x-coordinates where the window should 
be initialized 
y_start_chatWindow is the y-coordinate where the other window starts,
it is needed to calculate the height of the upper half of the screen */
static WINDOW * 
configureTextWindow(int y_start, int x_start, int y_start_delimiter)
{

	/* create separate window for chat input */
	WINDOW * textWindow;
	/* height is (y_start_chatWindow -1)
	width is COLS -2 */
	textWindow = newwin(y_start_delimiter-1,COLS-2,y_start,x_start);
	if(textWindow == NULL)	/* there was an error */
		errExit("newwin [textWindow]");

	return textWindow;
}

/* function to configure and create a window for the chat input (lower
half of screen)
y_start and x_start are the y,x-coordinates where the window should 
be initialized */
static WINDOW * 
configureChatWindow(int y_start, int x_start)
{

	/* create separate window for chat input */
	WINDOW * chatWindow;
	/* height is LINES-y_start
	width is COLS -2 */
	/* TODO: it is probably not that secure to use LINES and COLS here, since they are only initialized after 
	initializing ncurses*/
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

/* this window will print a line separating the chat and text windows */
static WINDOW *
configureDelimiterWindow(int y_start, int x_start, int height)
{
	/* create separate window for delimiter */
	WINDOW * delimiterWindow;
	delimiterWindow = newwin(height,COLS,y_start,x_start);
	if(delimiterWindow == NULL) /* there was an error */
		errExit("newwin [delimiterWindow]");
	
	/* draw a horizontal line to delimit the windows */
	if(whline(delimiterWindow,'_',COLS-2)!=OK)
		errExit("hline chatWindow");
	wrefresh(delimiterWindow);

	return delimiterWindow;

}

/* print messages received from server, all syscalls implemented inside this
function are NON-BLOCKING */
static void
printMessagesFromServer(WINDOW * window, int pipe_fd, int max_y_position)
{
	
	ssize_t bytesReceived;

	char * string_buf = (char *) malloc(BUF_SIZE);
	/* malloc failed */
	if(string_buf == NULL)
		errExit("malloc failed. @printMessagesFromServer");

	/* fetch messages from server into allocated string buffer */
	bytesReceived = fetchMessage(pipe_fd, string_buf);

	/* print messages, if received */
	if(bytesReceived > 0){
		/* check if the cursor is after the last line of the
		textWindow, in that case clear the window and move cursor
		to the origin to start printing messages from the top again */
		int y_cursor = getcury(window);
		if(y_cursor==ERR)
			errExit("y_cursor @printMessagesFromServer");
		if(y_cursor > max_y_position){
			/* clear screen and move back to origin */
			if(wclear(window)==ERR)
				errExit("wclear");
			if(wmove(window,0,0)==ERR)
				errExit("wmove");
		}
		wprintw(window,"%s\n",string_buf);
		wrefresh(window);
	}// end bytesReceived > 0

	/* if fetchMessage() failes, then bytesReceived < 0, it quietly
	frees memory, and returns, in order to try to fetch messages again 
	in the next iteration of the frontEnd main loop */

	/* free resources */
	free(string_buf);
}

static void 
handleNewline(WINDOW * chatWindow, int pipe_fd, const char * username_input)
{
	/* allocate memory locally to store text written in front-end */
	char * message = (char *) malloc(BUF_SIZE);
	/* malloc failed if message == NULL */
	if(message == NULL)
		errExit("malloc failed. handleNewline()");
	/* use memset to guarantee that all values of message
	are initialized with a 0,
	the 0 is important, since we are handling strings here, and some functions
	like strcat, expect a 0 to end a string, otherwise they are going to never 
	finish */
    memset(message, 0, BUF_SIZE);

	/* get current x-cursor position, to get number of characters to be
	sent (the current implementations only reads characters from
	one single line */
	int x_cursor = getcurx(chatWindow);
	if(x_cursor==ERR)
		errExit("getting x cursor position @handleNewline");
	/* copy up to max. characters at position, into message char array,
	the coordinates to start copying strings, should be the relative
	coordinates inside chatWindow, so 0,0 actually equals 
	y_start_chatWindow,x_start_chatWindow */
	int errorString = mvwinnstr(chatWindow,0,0,message,x_cursor);
	/* if error happens, close ncurses environment, to get a normal
	terminal and free memory */
	if(errorString == ERR){
		free(message);
		endwin();
		errExit("mvwinnstr [chatWindow]");
	}

	/* store the username received as parameter in the heap, in order to append
	to message sent back to server */
	char * username = (char *) malloc(BUF_SIZE);
	if(username == NULL)
		errExit("malloc failed. handleNewline()");
	/* guarantee that all values of username are initialized with 0 */
	memset(username, 0, BUF_SIZE);
	/* copy value of username in parameter into heap memory allocation */
	if(strcpy(username,username_input)!=username)
		errExit("strcpy");

	/* append message to username */
	if(strcat(username,message)!=username)
		errExit("strcat");

	/* append newline to username */
	const char * nl = "\n";
	/* append newline to message, before sending the message to the server */
	if(strcat(username, nl)!=username)
		errExit("strcat");
	
	/* send concatenation of username, message and newline just written to pipe, 
	to child process which sends message to server */
	sendMessageToPipe(pipe_fd, username);
	free(message);
	free(username);

	/* delete line which was just sent */
	if(wdeleteln(chatWindow)==ERR){
		endwin();
		errExit("delete line failed, after newline.");
	}
	/* after deleting line, move cursor back to origin */
	if(wmove(chatWindow,0,0)==ERR){
		endwin();
		errExit("move cursor to origin failed, after newline.");
	}
}

static void 
handleBackspace(WINDOW * chatWindow)
{

	/* get current x position (relative to chatWindow coordinates, so
	x=0, actually equals to x_start_chatWindow) */
	int x_position=getcurx(chatWindow);
	/* if cursor position is x=0, then do not do anything 
	(edge case). the coordinates are relative to chatWindow */
	if(x_position==0)
		return; /* do not continue deleting characters, since the 
					cursor is at the border of the window */
	/* move cursor to the left to delete last pressed character */
	x_position--;
	int y_position=getcury(chatWindow);
	wmove(chatWindow,y_position,x_position); /* move cursor to new position */
	/* delete character under cursor */
	wdelch(chatWindow);

}

/* this function checks if the un-sent message has achieved
the maximum size that can be handled, if not it returns the current
x position of the cursor (relative to the frame of reference of chatWindow)
if it has achieved the max size, then it returns -1 */
static int
checkMaxMessageLength(WINDOW * chatWindow, int maxMessageSize)
{
	/* variables to store current cursor position inside chatWindow */
	int y_cursor;
	int x_cursor;
	/* get the current cursor position */
	getyx(chatWindow,y_cursor,x_cursor);

	/* subtract 1 from maxMessageSize, since the first character start at x_cursor position 0 */
	if(x_cursor < (maxMessageSize -1))
		return x_cursor;

	/* return -1, since max capacity of message was achieved */
	return -1;

}

/* parse USERNAME from config file and append semicolon to username,
store username with suffix in same char * as parameter,
parse HOST and PORT as well */
static void
getConfigValues(char * username_parsed, char * port_parsed, char * host_parsed, char * key)
{

	/* allocate memory to store path of config file */
	char * client_config_file = (char *) malloc(1024);
	if(client_config_file==NULL)
		errExit("malloc failed");

	/* get HOME path of program's user */
	char * home_path = getenv("HOME");

	/* use strcpy and strcat to finish building path to client's config file */
	if(strcpy(client_config_file,home_path)!=client_config_file)
		errExit("strcpy");
	const char * config_file_suffix = "/.papayachat/client.config";
	if(strcat(client_config_file,config_file_suffix)!=client_config_file)
		errExit("strcat config file suffix");

	/* parse USERNAME in client's config file */
	if(parseConfigFile(client_config_file, "USERNAME", username_parsed)==-1)
		errExit("parseConfigFile for username failed");

	/* parse PORT in client's config file */
	if(parseConfigFile(client_config_file, "PORT", port_parsed)==-1)
		errExit("parseConfigFile for port failed");
	
	/* parse HOST in client's config file */
	if(parseConfigFile(client_config_file, "HOST", host_parsed)==-1)
		errExit("parseConfigFile for host failed");

	/* parse KEY in client's config file */
	if(parseConfigFile(client_config_file, "KEY", key)==-1)
		errExit("parseConfigFile for key failed");


	free(client_config_file);

	/* append semicolon and white-space to username */
	const char * username_suffix = ": ";
	if(strcat(username_parsed, username_suffix)!=username_parsed)
		errExit("strcat ': '");	

}

/* send authentication key to the server */
static void
sendAuthKey(int server_fd, char * key)
{
	
	/* the write() call should write exactly key_size bytes, otherwise
	it has failed */
	if(write(server_fd,key,KEY_LENGTH)!=KEY_LENGTH){
		errExit("key auth write() failed: ");
	}

}

int 
main(int argc, char *argv[])
{

/*-------------------Parse config values-------------------------------------------------*/
	/* allocate memory to store USERNAME, PORT and HOST values after being parsed,
	MAX_LINE_LENGTH is the maximum amount of characters that will be parsed
	per line */
	char * username_parsed = (char *) malloc(MAX_LINE_LENGTH+10);
	/* if malloc fails, it returns a NULL pointer */
	if(username_parsed == NULL)
		errExit("malloc username_parsed failed");
	char * port_parsed = (char *) malloc(MAX_LINE_LENGTH+10);
	/* if malloc fails, it returns a NULL pointer */
	if(port_parsed == NULL)
		errExit("malloc port_parsed failed");
	char * host_parsed = (char *) malloc(MAX_LINE_LENGTH+10);
	/* if malloc fails, it returns a NULL pointer */
	if(host_parsed == NULL)
		errExit("malloc host_parsed failed");
	/* get key from key file */
	char * key = (char *) malloc(KEY_LENGTH);
	if(key==NULL)
		errExit("malloc key failed");

	/* parse username, port and key from config file */
	getConfigValues(username_parsed,port_parsed,host_parsed,key);

/*---------------------------------------------------------------------------------------*/

	/* establish connection with server, get fd to be shared with child processes */
	int server_fd = establishConnection(host_parsed,port_parsed);
	/* if establishConenction fails, the program exits from within the function call
	to establishConnection, if the authentication fails, the connection would be closed much later, that
	is why it actually crashes when ncurses has already taken over control of the terminal */

	/* Send authentication key to server */
	sendAuthKey(server_fd,key);

	/* SECURITY: if there is core dump here I want the memory block where the key was to be all 0s, so that
	the key is not leaked, check 'The Linux Programming Interface' for a similar procedure */
	memset(key,0,KEY_LENGTH);
	/* free variables not needed any more */
	free(key);
	free(host_parsed);
	free(port_parsed);

	/* configure catching SIGCHLD of child processes */
	if(configureSignalDisposition()==-1)
		errExit("sigaction failed, at configureSignalDisposition");

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

	int child1_pid = fork();
	/* 1. Child creation -- handles sending data from server */
	switch(child1_pid) {
		/* error on fork() call  */
		case -1:
			/* exit with an error message */
			errExit("fork(), 1. Child [send] could not be created.");
			break;

		/* 1. Child process (returns 0) */
		case 0: 
			/* Close stdin of pipe shared with parent */
			close(pipe_fds_send_server[1]);
			handleSendSocket(server_fd, pipe_fds_send_server[0]);
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
	/* transform fd to non-blocking */	
	int flags;
	/* get currently used flags on pipe */
	flags = fcntl(pipe_fds_receive_server[0], F_GETFL); 
	if(flags==-1)
		errExit("fcntl F_GETFL");
	/* activate O_NONBLOCK flag */
	flags |= O_NONBLOCK;	
	/* configure pipe fd to be O_NONBLOCK */
	if(fcntl(pipe_fds_receive_server[0],F_SETFL,flags)==-1)
		errExit("fcntl O_NONBLOCK");

	int child2_pid = fork();
	switch(child2_pid) {
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
			/* this function call will continously read from server socket
			if no messages are being received, it will block, if server 
			disconnects then it will exit with an error */
			handleReadSocket(server_fd, pipe_fds_receive_server[1]);
			_exit(EXIT_SUCCESS);
			break;

		/* Parent: fork() returns PID of newly created child */
		default:
			/* Close stdin of pipe shared with 2. Child */
			close(pipe_fds_receive_server[1]);
			break;
	}// end switch-case fork 2

	
	/* initialize and configure ncurses */
	configureNcurses();

	/* configure only the parent process to kill all children when invoking exit(3) */
	if(atexit(killChildProcesses)== -1)
		errExit("atexit");


	/* Print 'papayaChat' in the first 0, right in the middle of the screen 
	subtract half of the length of 'papayaChat' from the x position in the
	middle of the screen */
	mvprintw(0,COLS/2-strlen("papayaChat")/2,"papayaChat\n");
	attron(COLOR_PAIR(INSTRUCTIONS_COLOUR));
	mvprintw(1,0,">>Press ARROW_DOWN to exit chat<<\n");
	attroff(COLOR_PAIR(INSTRUCTIONS_COLOUR));
	/* HLINE not printing */
	////if(hline('_',COLS)==ERR)
		////errExit("hline1");
	
	////printw("\n");
	refresh();

	int a = 0;			/* TODO: uninitialized ERROR found here fix that*/

	/* starting position for chatWindow */
	int x_start_chatWindow = 1;
	int y_start_chatWindow = LINES-4; /* 4 lines below the last line */
	/* starting position for textWindow */
	int x_start_textWindow = 1;
	int y_start_textWindow = 3; /* on the 4th vertical line form the top */
	
	int y_start_delimiter = y_start_chatWindow - 2;	
	int max_y_textWindow = LINES/2;

	/* create chatWindow */
	WINDOW * chatWindow;
	chatWindow=configureChatWindow(y_start_chatWindow,x_start_chatWindow);

	/* create textWindow */
	WINDOW * textWindow;
	textWindow=configureTextWindow(y_start_textWindow,x_start_textWindow,y_start_delimiter);
	
	/* move the cursor on textWindow to the position (0,0) */
	wmove(textWindow,0,0);

	/* Stop ncurses when 'KEY_DOWN' is pressed */
	while(a!= KEY_DOWN){
		a = wgetch(chatWindow);
		/* if getch returns ERR then no character was typed,
		getch was configured as non-blocking with the nodelay() function */
		if(a != ERR){
			/* carriage return was pressed, send line to server, delete
			line and move cursor to origin */
			if(a == '\n'){
				handleNewline(chatWindow,pipe_fds_send_server[1], username_parsed);	
				/* newline was handled, continue trying to read input from keyboard */
				continue;
			} // if-statement \n (newline)

			/* BACKSPACE was pressed, delete last character */
			if(a == PORTABLE_BACKSPACE){
				handleBackspace(chatWindow);
				/* backspace was handled, continue trying to read input from keyboard */
				continue;
			}
			/* if the current cursor position is smaller than the max. message length,
			add new read character to chatWindow and refresh view of chatWindow */
			int messageMaxLength = MAX_MESSAGE_SENT;
			if(checkMaxMessageLength(chatWindow,messageMaxLength)!=-1){
				waddch(chatWindow,a);
				wrefresh(chatWindow);
			}

		} // end a!=ERR
	/* fetch new messages from server (if available, otherwise nothing happens) */
	printMessagesFromServer(textWindow,pipe_fds_receive_server[0],max_y_textWindow);

	} // while-loop

	/* End ncurses, KEY_DOWN was pressed */
	endwin();			

	free(username_parsed);
	exit(EXIT_SUCCESS);
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
