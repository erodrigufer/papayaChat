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

/* function to send new messages to client after receiving SIGUSR1 signal */
static void
sendNewMessages(int client_fd, int chatlog_fd)
{
	syslog(LOG_DEBUG, "value of flag_activated before SIGUSR1= %d", flag_activated);
	/* activate SIGUSR1 only for this child process */
	if(activateSIGUSR1()==-1){
		syslog(LOG_ERR, "activateSIGUSR1() failed: %s", strerror(errno));
		/* TODO: actually if something fails, then there should be a defered function
		which also sends a kill SIGTERM to all processes in the group, or at least to 
		all the processes handling the same client */
		_exit(EXIT_FAILURE);
	}// end activateSIGUSR1()

	/* start reading from beginning of file */
	off_t offset = 0;

	for(;;){
		pause();			

		/* TODO: handle more properly with malloc() */
		char buffer[BUF_SIZE];

		/* just a debug run, it should not exit here */
		syslog(LOG_DEBUG, "value of flag_activated= %d", flag_activated);
	
		ssize_t bytesRead = sharedRead(chatlog_fd, buffer, BUF_SIZE, offset);
		if(bytesRead==-1){
			syslog(LOG_ERR, "sharedRead() failed: %s", strerror(errno));
			_exit(EXIT_FAILURE);
			}

		/* update offset value after read */
		offset = offset + bytesRead;

		/* DEBUG: print to syslog the contents of the chat log */
		syslog(LOG_DEBUG, "---> Contents of chat log: %s<---", buffer);

	}// end for-loop
}

/* send a SIGTERM signal when closing connection or crashing with error to child process
(the process sending the messages back to the client) */
static void
killChild(pid_t child_pid)
{
	/* TODO: in theory this system call could fail, and it would return -1
	in that case we weould have to do something more aggresive, like kill
	all processes in the process group */
	kill(child_pid, SIGTERM);

}

/* receive messages from client and write them exclusively (using file locks)
into the chat log file */
static void 
receiveMessages(int client_fd, int chatlog_fd, pid_t child_pid)
{

	/* TODO: in theory this should be handled more properly with malloc() */
    char buf[BUF_SIZE];
    ssize_t numRead;

	/* if the client closes its connection, the previous read() syscall will get an
	EOF, and it will return 0, in that case, the while-loop ends, and there is no 
	syslog error appended to the log, since read() did not return an error */  
	while ((numRead = read(client_fd, buf, BUF_SIZE)) > 0) {
        if (write(client_fd, buf, numRead) != numRead) {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
			killChild(child_pid);
            _exit(EXIT_FAILURE);
        }
		/* add debug syslog to see amount of bytes received from client */
		syslog(LOG_DEBUG, "%ld Bytes received from client.", numRead);

		/* using locks guarantee exclusive write on file with concurrent clients */
		if(exclusiveWrite(chatlog_fd, buf, numRead)==-1){
			syslog(LOG_ERR, "exclusiveWrite() failed: %s", strerror(errno));
			killChild(child_pid);
			_exit(EXIT_FAILURE);
		}
    } // while-loop read()

    if (numRead == -1) {
        syslog(LOG_ERR, "read() failed: %s", strerror(errno));
		killChild(child_pid);
        _exit(EXIT_FAILURE);
    }

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

		/* Parent process */
		default:
			receiveMessages(client_fd, chatlog_fd, sendingChild_pid);
		
	} // end switch-statement

}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
