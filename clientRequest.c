/* clientRequest.c 

[Server-side functions]
Functions to handle sending and receiving data from clients.

*/

#include <signal.h>		/* needed for sig_atomic_t variable */

#include <syslog.h>		/* server runs as daemon, pipe errors messages to syslog */
/* daemon posts still with the configuration of concurrent_server.c, 
configure_syslog.h functions are unnecessary */
#include "signalHandling.h"
#include "clientRequest.h"
#include "basics.h"
#include "file_locking.h"
#include "CONFIG.h"	/* declaration of BUF_SIZE */

/* global (extern) variable from signalHandling.c 
this variable is 0 before SIGUSR1 is received, and it changes to 1, when SIGUSR1 is received */
extern volatile sig_atomic_t flag_activated;

/* define greetingMessage string, the compiler allocates enough memory for the string */
const char greetingMessage [] = "\
\n\
                  ...papayaChat...\n\n\
papayaChat is licensed under GNU AGPLv3.\n\
The code is hosted at: www.github.com/erodrigufer/papayaChat\n\
--------------------------------------------------------------\n\
* Press DOWN ARROW to leave the chat\n\
\n\
";

/* send greetings to client when initializing information exchange */
static void
introMessage(int client_fd)
{

	/* the size of the string is calculated statically at compile time,
	check 'Effective C' page 133 */
	size_t greetingSize = sizeof greetingMessage;
	
	/* the write() call should write exactly greetingSize bytes, otherwise
	it has failed */
	if(write(client_fd,greetingMessage,greetingSize)!=greetingSize){
		syslog(LOG_ERR, "write() failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}
 
}

/* helper function used to read from chatlog file and to send its
contents directly to the client */
static off_t
readChatlogSendClient(int client_fd, int chatlog_fd, off_t offset)
{
	/* allocate memory on each for-loop to read message
	from pipe */
	char * string_buf = (char *) malloc(BUF_SIZE);
	/* if malloc fails, it returns a NULL pointer */
	if(string_buf == NULL){
		syslog(LOG_ERR, "malloc failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}
	/* guarantee that string_buf has 0 value */
	memset(string_buf,0,BUF_SIZE);

	/* shared read (safe read) from chatlog file */	
	ssize_t bytesRead = sharedRead(chatlog_fd, string_buf, BUF_SIZE, offset);
	if(bytesRead==-1){
		syslog(LOG_ERR, "sharedRead() failed: %s", strerror(errno));
		/* read failed, free malloc resources before exiting */
		free(string_buf);
		_exit(EXIT_FAILURE);
	}

	/* the sharedRead delivered EOF, so there were no new messages to read
	do not change the offset and return immediately */
	if(bytesRead==0){
		free(string_buf);
		return offset;
	}

	/* TODO: consider that all of this is happening after UNLOCKING
	shared read lock */
	/* ANSWER: this is a problem, MAYBE, because another client might send new messages 
	to the server, but in that case I think that another SIGUSR would be triggered, and the
	system would start reading again from the chatlog */

	/* update offset value after read */
	offset = offset + bytesRead;

	/* store only the characters read on a new string */
	char * stringClient = (char *) malloc(bytesRead);
	/* if malloc fails, it returns a NULL pointer */
	if(stringClient == NULL){
		syslog(LOG_ERR, "malloc failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}
	/* copy only the bytesRead into stringClient */
	if(snprintf(stringClient,bytesRead,"%s",string_buf)<0){
		syslog(LOG_ERR, "snprintf() failed: %s", strerror(errno));
		free(string_buf);
		free(stringClient);
		_exit(EXIT_FAILURE);
	}

	/* send message to client socket */
	if(write(client_fd,stringClient,bytesRead)!=bytesRead){
		syslog(LOG_ERR, "write() failed: %s", strerror(errno));
		free(string_buf);
		free(stringClient);
		_exit(EXIT_FAILURE);
	}

	/* DEBUG: print to syslog the contents of the chat log */
	//syslog(LOG_DEBUG, "---> Contents of chat log: %s<---", stringClient);
	
	/* free malloc resources before end of loop */
	free(string_buf);
	free(stringClient);

	return offset; /* value used in the next iteration */

}

/* function to send new messages to client after receiving SIGUSR1 signal */
static void
sendNewMessages(int client_fd, int chatlog_fd)
{
	/* activate SIGUSR1 only for this child process, this means that this child process
	can receive the SIGUSR1 signal, when a client sends a message to the server */
	if(activateSIGUSR1()==-1){
		syslog(LOG_ERR, "activateSIGUSR1() failed: %s", strerror(errno));
		/* TODO: actually if something fails, then there should be a defered function
		which also sends a kill SIGTERM to all processes in the group, or at least to 
		all the processes handling the same client */
		_exit(EXIT_FAILURE);
	}// end activateSIGUSR1()

	/* start reading from beginning of file */
	off_t offset = 0;

	/* read from chatlog as soon as the client connects, and send
	new messages eventually right away */
	offset = readChatlogSendClient(client_fd, chatlog_fd, offset);

	//syslog(LOG_DEBUG, "value of flag_activated before SIGUSR1= %d", flag_activated);

	
	for(;;){
		/* block until a signal is received, in this case the multicast SIGUSR1 */
		pause();			
		
		/* SIGUSR1 was received, so attempt to read from chatlog and send new messages to client */
		offset = readChatlogSendClient(client_fd, chatlog_fd, offset);


	}// end for-loop
}

/* send a SIGTERM signal when closing connection or crashing with error to child process
(the process sending the messages back to the client) */
static void
killChild(pid_t child_pid)
{
	/* TODO: in theory this system call could fail, and it would return -1
	in that case we would have to do something more aggresive, like kill
	all processes in the process group */
	if(kill(child_pid, SIGTERM)==-1){
		syslog(LOG_ERR, "kill child process failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}
	syslog(LOG_DEBUG, "Killed child process PID: %d.", child_pid);

}

/* receive messages from client and write them exclusively (using file locks)
into the chat log file */
static void 
receiveMessages(int client_fd, int chatlog_fd, pid_t child_pid)
{


	for(;;) {
    	ssize_t numRead;
		/* allocate memory on each for-loop to read message
		from pipe */
		char * buf = (char *) malloc(BUF_SIZE);
		/* if malloc fails, it returns a NULL pointer */
		if(buf == NULL){
			syslog(LOG_ERR, "malloc failed: %s", strerror(errno));
			_exit(EXIT_FAILURE);
		}

		/* if the client closes its connection, the previous read() syscall will get an
		EOF, and it will return 0, in that case, the while-loop ends, and there is no 
		syslog error appended to the log, since read() did not return an error */  
		if ((numRead = read(client_fd, buf, BUF_SIZE)) > 0) {
			/* add debug syslog to see amount of bytes received from client */
			syslog(LOG_DEBUG, "%ld Bytes received from client.", numRead);

			/* using locks guarantee exclusive write on file with concurrent clients */
			if(exclusiveWrite(chatlog_fd, buf, numRead)==-1){
				syslog(LOG_ERR, "exclusiveWrite() failed: %s", strerror(errno));
				free(buf);
				killChild(child_pid);
				_exit(EXIT_FAILURE);
			}
		} // read()

		/* free resources */
		free(buf);

		if (numRead == -1) {
			syslog(LOG_ERR, "read() failed: %s", strerror(errno));
			killChild(child_pid);
			_exit(EXIT_FAILURE);
		}

		/* EOF - client closed socket */
		if (numRead == 0){
			syslog(LOG_DEBUG, "Received EOF from client!");
			break; /* break out of for-loop after EOF */
		}
	}//infinite for-loop

	/* when the connection is closed EOF is received, the for-loop stops, and the functions 
	just returns with no value (void), therefore we should also kill the child processes here */
	killChild(child_pid);

	/* kill  receiveMessages() child process, after this command, all child processes created for
	a particular client should be gone!! this fixes the bug present in pre-release v0.1.0-alpha */
	_exit(EXIT_SUCCESS);

}

/* Handle a client request: copy socket input back to socket */
void
handleRequest(int client_fd, int chatlog_fd)
{
	/* send intro message to client */
	introMessage(client_fd);

	/* store the pid of the child process in order to send kill signal when connection is closed */
	pid_t sendingChild_pid = fork();
	/* create a new child process to solely handle sending new messages back to client */
	switch(sendingChild_pid) {
		/* error */
		case -1:
			syslog(LOG_ERR, "Error fork() call. Can't create child (%s)", strerror(errno));
			_exit(EXIT_FAILURE);

		/* Child process */
		case 0:
			sendNewMessages(client_fd, chatlog_fd);
			/* when the parent process receiveMessages() receives a EOF from the client, when the client
			disconnects, then the parent process sends a SIGTERM signal to the child (sendNewMessages) 
			and _exit() itself. Therefore finishing all processes of a particular client. The sockets used
			are closed when the processes exit. */

		/* Parent process */
		default:
			receiveMessages(client_fd, chatlog_fd, sendingChild_pid);
		
	} // end switch-statement

}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
