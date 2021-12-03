/* clientRequest.c 

[Server-side functions]
Functions to handle sending and receiving data from clients.

*/

#include <syslog.h>		/* server runs as daemon, pipe errors messages to syslog */
/* daemon posts still with the configuration of concurrent_server.c, 
configure_syslog.h functions are unnecessary */
#include "signalHandlingh."
#include "clientRequest.h"
#include "basics.h"
#include "file_locking.h"
#include "CONFIG.h"	/* declaration of BUF_SIZE */

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


	for(;;){
		pause();			
		/* just a debug run, it should not exit here */
		syslog(LOG_DEBUG, "value of flag_activated= %d", flag_activated);
		_exit(EXIT_SUCCESS);
	}// end for-loop
}

/* receive messages from client and write them exclusively (using file locks)
into the chat log file */
static void 
receiveMessages(int client_fd, int chatlog_fd)
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
            _exit(EXIT_FAILURE);
        }
		/* add debug syslog to see amount of bytes received from client */
		syslog(LOG_DEBUG, "%ld Bytes received from client.", numRead);

		/* using locks guarantee exclusive write on file with concurrent clients */
		if(exclusiveWrite(chatlog_fd, buf, numRead)==-1){
			syslog(LOG_ERR, "exclusiveWrite() failed: %s", strerror(errno));
			_exit(EXIT_FAILURE);
		}
    } // while-loop read()

    if (numRead == -1) {
        syslog(LOG_ERR, "read() failed: %s", strerror(errno));
        _exit(EXIT_FAILURE);
    }

}

/* Handle a client request: copy socket input back to socket */
void
handleRequest(int client_fd, int chatlog_fd)
{
	/* send intro message to client */
	introMessage(client_fd);

	/* create a new child process to solely handle sending new messages back to client */
	switch(fork()) {
		/* error */
		case -1:
			syslog(LOG_ERR, "Error fork() call. Can't create child (%s)", strerror(errno));
			_exit(EXIT_FAILURE);

		/* Child process */
		case 0:
			sendNewMessages(client_fd, chatlog_fd);

		/* Parent process */
		default:
			receiveMessages(client_fd, chatlog_fd);
		
	} // end switch-statement

}


/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
