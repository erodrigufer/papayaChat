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

/* Load TCP/IP services from CONFIG.h header */
#include "CONFIG.h" /* file defines IP and port of chat service server */

#define BUF_SIZE 4096 /* bytes transmission size */

/* Define this values as global to be able to call them from the atexit() function */
int child1_pid;
int child2_pid;

/* run this function to catch SIGCHLD of child processes exiting */
static void
catchSIGCHLD(int sig)
{
	int savedErrno;             /* Save 'errno' in case changed here, errno
									can be changed by waitpid() if no more children exist
									then 'errno'= ECHILD */
		savedErrno = errno;
		while (waitpid(-1, NULL, WNOHANG) > 0)	/* 'man 2 waitpid' -1 means that it waits 
		for any of its child processes, the option WNOHANG makes the syscall waitpid()
		to return inmediately if no children has exited, if children exist but none has changed 
		status, then waitpid returns 0, so it exits the while-loop (on error, it returns -1 and
		also exits the while-loop). In any other case, multiple children have exited, so waitpid()
		will return their pids (which are larger than 0). After the last children which has changed
		status, waitpid will return either 0 or -1 and the while-loop will come to an end
		If there are no more children, then waitpid will return -1 and set errno to ECHILD */
			continue;
		errno = savedErrno;			/* restore errno to value before signal handler */

}

/* run this function to kill child processes */
static void
killChildProcesses(void)
{
	/* IMPORTANT: I should not use errExit with these functions
	since they are already being run atexit(), so only display 
	diagnostic error messages */
	/* kill 1. Child */
	if(kill(child1_pid,SIGTERM)==-1)
		errMsg("Failed to kill 1. Child!");
	
	/* kill 2. Child */
	if(kill(child2_pid,SIGTERM)==-1)
		errMsg("Failed to kill 2. Child!");

/* TODO: debugging purpouses, remove fprintf later */
	fprintf(stderr,"Successfully killed child processes!\n");

}

/* establish TCP connection with server, return fd of server socket */
static int 
establishConnection(void){
	int server_fd; /* fd for server connection */

	/* SOCK_STREAM for TCP connection */
	server_fd = clientConnect(HOST,SERVICE,SOCK_STREAM);
	if(server_fd == -1)
		errExit("clientConnect"); /* connection failed, exit */

	return server_fd;
}

/* function to configure and initialize the ncurses environment */
static void 
configureNcurses(void)
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

	/* TODO: remove this comments if the window partitioning works out */
	//keypad(chatWindow, TRUE); /* Enable keypad and F-keys */

	/* stdin reading should be non-blocking, if no character is read from
	stdin, then getch returns ERR 
	Otherwise, if it weren't non-blocking, we would not be able to 
	print in the upper half of the chat window concurrently */
	//if(nodelay(chatWindow,TRUE)==ERR)
		//errExit("nodelay[chatWindow]");		/* nodelay() failed, catastrophic error */

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
/* send a message to a pipe */
static void
sendMessageToPipe(int pipe_fd, char *message)
{
	ssize_t bytesWritten;
	/* Execute strlen outside of write(), since it demands run time resources
	and it would be run twice inside write() call*/
	size_t sizeOfMessage = strlen(message);
	if(write(pipe_fd,message,sizeOfMessage)!=sizeOfMessage)
		errExit("write sendMessageToPipe()");

}

/* send a message to the server_fd, the message is received through 
a pipe from the parent process */
static void
handleSendSocket(int server_fd, int pipe_fd)
{
	/* size of info read from pipe */
	ssize_t bytesRead;

	for(;;){

		/* allocate memory on each for-loop to read message
		from pipe */
		char * string_buf = (char *) malloc(BUF_SIZE);
		/* if malloc fails, it returns a NULL pointer */
		if(string_buf == NULL)
			errExit("malloc failed. @handleSendSocket()");

		/* read from pipe info to send to server */
		bytesRead = read(pipe_fd, string_buf, BUF_SIZE);
/*----------- error handling for read()----------------------------------- */
		/* read() failed, exit programm with error, always free malloc resources
		before exit()*/
		if(bytesRead == -1){
			free(string_buf);
			errExit("read - handleSendSocket()");
		}
		/* connection to pipe closed */
		if(bytesRead == 0){
			free(string_buf);
			errExit("pipe closed - handleSendSocket()");
		}
/*----------- error handling for read()----------------------------------- */
		/* send data received from pipe to server socket */
		if(write(server_fd,string_buf,bytesRead)!=bytesRead)
			errExit("write handleSendSocket()");

		/* free resources, char * buffer to read message from pipe */
		free(string_buf);
	} // end for-loop

}

/* fetch messages from pipe (fetch, but do not print message yet)
the pipe should be configured as O_NONBLOCK
since if the read() on the pipe blocks, all the CLI stalls */
static int
fetchMessage(int pipe_fd, char *string_buf)
{
	ssize_t bytesRead;

	bytesRead = read(pipe_fd, string_buf, BUF_SIZE);

/* TODO: handle errors from read() */

	/* reading from pipe failed */
//	if(bytesRead == -1)
		//errMsg("read greetings message");

	return bytesRead;

}

/* print messages received from server */
static void
printMessagesFromServer(WINDOW * window, int pipe_fd)
{
	
	ssize_t bytesReceived;

	char * string_buf = (char *) malloc(BUF_SIZE);
	/* malloc failed */
	if(string_buf == NULL)
		errExit("malloc failed. @printMessagesFromServer");

	/* fetch messages from server into allocated string buffer */
	bytesReceived = fetchMessage(pipe_fd, string_buf);

	/* TODO: handle errors from fetchMessage */

	/* print messages, if received */
	if(bytesReceived > 0){
		wprintw(window,"%s\n",string_buf);
		wrefresh(window);
	}

	/* free resources */
	free(string_buf);
}


/* read from server socket and pass data through pipe to frontEnd parent process */
static void
handleReadSocket(int server_fd, int pipe_fd)
{

	ssize_t bytesRead;

/* the right way to handle strings is by allocating new memory each time
a string is received through a read() call from the server, since 
char * should be immutable objects in C */
	for(;;){

		/* TODO: check if the size allocated is correct */
		/* allocate memory on each for-loop to read from server, into
		newly allocated char* buffer */
		char * string_buf = (char *) malloc(BUF_SIZE);
		/* if malloc fails, it returns a NULL pointer */
		if(string_buf == NULL)
			errExit("malloc() failed. @handleReadSocket()");
		
		bytesRead = read(server_fd, string_buf, BUF_SIZE);
/*----------- error handling for read()----------------------------------- */
		/* read() failed, exit programm with error, always free malloc resources
		before exit()*/
		if(bytesRead == -1){
			free(string_buf);
			errExit("read from server @handleReadSocket()");
		}
		/* connection to server down */
		if(bytesRead == 0){
			free(string_buf);
			errExit("connection to server lost! read() from socket return 0 == EOF :@handleReadSocket()");
		}
/*----------- error handling for read()----------------------------------- */
		/* send data received from server to parent process through pipe */
		if(write(pipe_fd, string_buf, bytesRead) != bytesRead){
			/* the amount of bytes written is not equal to the amount of bytes read */
			errExit("write to pipe @handleReadSocket()");
		}
		/* free resources, char * buffer to read message from server */
		free(string_buf);
	}
}

static void
configureSignalDisposition(void)
{

    struct sigaction sa_sigchild;			/* struc is necessary to define signals mask
											to be blocked during signal handler, needed 
											for syscall sigaction*/

    /* Establish SIGCHLD handler to reap terminated child processes,
	if SIGCHLD is gathered with waitpid() or wait() by parent, then child
	process becomes a zombie and resources (PIDs) are not used efficiently.
	SIGCHLD is sent by the kernel to a parent process when one of its childern terminates
	(either by calling exit() or as a result of being killed by a signal).

	sa_mask is the signal set of signals that would be blocked during the
	invocation of the handler
	-> create an empyte signal set, no signal blocked during invocation of handler */
    sigemptyset(&sa_sigchild.sa_mask);			

	/* if a syscall is interrupted by the SIGCHLD, the kernel should
	restart the syscall after handling the signal,
	for that the SA_RESTART flag is used. Not all syscalls can be
	properly restarted by the kernel, check 21.5 of 'The Linux 
	Programming Interface' 
	for the SIGTERM handler this is not required, since all kernel syscalls
	interrupted will not matter since the process should exit ASAP */
    sa_sigchild.sa_flags = SA_RESTART;
	/* catchSIGCHLD is the function handler for a SIGCHLD signal */
    sa_sigchild.sa_handler = catchSIGCHLD;	

	/* the new disposition for SIGCHLD signal is the grimReaper function, the old
	signal disposition is not stored anywhere (NULL) */
    if (sigaction(SIGCHLD, &sa_sigchild, NULL) == -1)
		errExit("sigaction for SIGCHLD");

}

static void 
handleNewline(WINDOW * chatWindow, int pipe_fd)
{
	/* allocate memory locally, and free memory after \n if statement */
	char * message = (char *) malloc(BUF_SIZE);
	/* malloc failed if message == NULL */
	if(message == NULL)
		errExit("malloc failed. Newline message send");

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
	int errorString = mvwinnstr(chatWindow,0,0,message,x_cursor+1);
	if(errorString == ERR){
		free(message);
		endwin();
		errExit("mvwinnstr [chatWindow]");
		}
	/* send message just written to pipe, to child process which
	sending message to server */
	sendMessageToPipe(pipe_fd, message);
	free(message);
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

int 
main(int argc, char *argv[])
{
	/* establish connection with server, get fd to be shared with child processes */
	int server_fd;
	server_fd = establishConnection();

	/* configure catching SIGCHLD of child processes */
	configureSignalDisposition();

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

	child1_pid = fork();
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

	child2_pid = fork();
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

	/* configure the parent process to kill all children when invoking exit(3) */
	if(atexit(killChildProcesses)== -1)
		errExit("atexit");


	/* Print 'papayaChat' in the first 0, right in the middle of the screen 
	subtract half of the length of 'papayaChat' from the x position in the
	middle of the screen */
	mvprintw(0,COLS/2-strlen("papayaChat")/2,"papayaChat\n");

	/* HLINE not printing */
	if(hline('_',COLS)==ERR)
		errExit("hline1");
	
	printw("\n");
	refresh();

/* TODO: To solve the problem of not displaying UTF, probably change a to int64_u */
	int a = 0;			/* TODO: uninitialized ERROR found here fix that*/

	/* starting position for chatWindow */
	int x_start_chatWindow = 1;
	int y_start_chatWindow = (int) LINES*4/5;
	/* starting position for textWindow */
	int x_start_textWindow = 1;
	int y_start_textWindow = 5;
	/* starting position for delimiting line */
	int x_start_delimiter = 0;
	int height_delimiter = 3;
	int y_start_delimiter = y_start_chatWindow - height_delimiter;
	
	//move(y_start_chatWindow-1,0); /* move cursor to start position */

	/* Draw horizontal line */
	//if(hline('_',COLS)==ERR)
	//	errExit("hline2");
	/* refresh() is needed to depict horizontal line */
	//refresh();

	/* create chatWindow */
	WINDOW * chatWindow;
	chatWindow=configureChatWindow(y_start_chatWindow,x_start_chatWindow);

	/* create textWindow */
	WINDOW * textWindow;
	textWindow=configureTextWindow(y_start_textWindow,x_start_textWindow,y_start_delimiter);

	/* create delimiterWindow */
	WINDOW * delimiterWindow;
	delimiterWindow=configureDelimiterWindow(y_start_delimiter,x_start_delimiter,height_delimiter);

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
				handleNewline(chatWindow,pipe_fds_send_server[1]);	
				/* newline was handled, continue trying to read input from keyboard */
				continue;
			} // if-statement \n (newline)

			/* BACKSPACE was pressed, delete last character */
			if(a == KEY_BACKSPACE || a == KEY_LEFT){
				handleBackspace(chatWindow);
				/* backspace was handled, continue trying to read input from keyboard */
				continue;
			}
			/* if the current cursor position is smaller than the max. message length,
			add new read character to chatWindow and refresh view of chatWindow */
			int messageMaxLength = COLS/2;
			if(checkMaxMessageLength(chatWindow,messageMaxLength)!=-1){
				waddch(chatWindow,a);
				wrefresh(chatWindow);
			}

		} // end a!=ERR
	/* fetch new messages from server (if available, otherwise nothing happens) */
	printMessagesFromServer(textWindow,pipe_fds_receive_server[0]);

	} // while-loop

	/* End ncurses, KEY_DOWN was pressed */
	endwin();			

	exit(EXIT_SUCCESS);
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
