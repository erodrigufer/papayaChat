#include <stdio.h>
#include <fcntl.h>   /* change fds to O_NONBLOCK */
#include <ncurses.h> /* required to create terminal UI */
#include <string.h>	/* required for string manipulation */
#include <signal.h>				/* check 'man 2 sigaction' signal.h is needed
								to change the disposition of signals with
								the sigaction() syscall */	
#include <sys/wait.h>	/* wait on child processes */

#include "basics.h" /* includes library to handle errors */
#include "inet_sockets.h" /* include library to handle TCP sockets */

/* Define TCP/IP services */
#define SERVICE "51000" /* Port to connect to with server */
#define HOST "localhost" /* server address (found, e.g. on /etc/hosts file) */
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

/* function to configure and create a window for the chat (lower
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

/* send a message to a pipe */
static void
sendMessageToPipe(int pipe_fd, char *message)
{

	ssize_t bytesWritten;
	if(write(pipe_fd,message,strlen(message)!=strlen(message)))
		errExit("write sendMessageToPipe");

}

/* send a message to the server_fd
the message is received through a pipe from the 
parent process */
static void
handleSendSocket(int server_fd, int pipe_fd, char *message)
{

	ssize_t bytesRead;
	char string_buf[BUF_SIZE];
	/* TODO: allocate memory with malloc,
	since otherwise always writing on top of string buf 
	is that permitted ? pipe will block when not receiving data 
	it will not return 
	then maybe create non-blocking pipe, return and re-allocate
	memory with malloc and free() */
	while((bytesRead = read(pipe_fd, string_buf, BUF_SIZE)) > 0){
		if(write(string_buf,
	}


}

/* Get the standard greetings message from the pipe */
static void
getGreetingsMessage(int pipe_fd, char *string_buf)
{
	ssize_t bytesRead;

	while((bytesRead = read(pipe_fd, string_buf, BUF_SIZE)) > 0){
	}
	/* reading from pipe failed */
//	if(bytesRead == -1)
		//errMsg("read greetings message");

}

static void
handleReadSocket(int server_fd, int pipe_fd)
{

	ssize_t bytesRead;
	char string_buf[BUF_SIZE];

	/* endless for-loop reading from the TCP-socket
	every time a whole message is read, then read() returns 0
	and the while-loop will then re-start with a blocking read()
	until some bytes can be read from the socket */
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
/* read from server socket and pass data through pipe to frontEnd parent process */
static void
handleReadSocket2(int server_fd, int pipe_fd)
{

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
		//if(bytesRead == 0)
		//	errExit("Connection to server closed");
	}
}

void
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
	/* 1. Child creation -- handles receiving data from server */
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
			handleReadSocket2(server_fd, pipe_fds_receive_server[1]);
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
	/* fetch new possible greetings messages */
	
	char string_buf[BUF_SIZE];

	getGreetingsMessage(pipe_fds_receive_server[0],string_buf);

	mvprintw(7,0,"%s",string_buf);
	refresh();
    }

	endwin();			/* End ncurses */

	printf("%s\n",message);
	exit(EXIT_SUCCESS);
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
