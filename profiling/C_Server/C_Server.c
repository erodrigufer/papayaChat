/* C_Server.c

Back-end server profilling 

*/
#include <signal.h>				/* check 'man 2 sigaction' signal.h is needed
								to change the disposition of signals with
								the sigaction() syscall */	
#include <sys/wait.h>
/* libraries needed to print the pid of a process, */
#include <sys/types.h>
#include <unistd.h>
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */
#include "basics.h"
#include "signalHandling.h"		/* signal handlers library */

#define BACKLOG_QUEUE 100		

/* bytes transmission size, defined in CONFIG.h
to share the value between multiple files */
#define BUF_SIZE 4096 


/* receive messages from client and write them exclusively to STDOUT */
static void 
receiveMessages(int client_fd)
{

	for(;;) {
    	ssize_t numRead;
		/* allocate memory on each for-loop to read message
		from pipe */
		char * buf = (char *) malloc(BUF_SIZE);
		/* if malloc fails, it returns a NULL pointer */
		if(buf == NULL){
			errExit("malloc failed.");
		}

		/* if the client closes its connection, the previous read() syscall will get an
		EOF, and it will return 0, in that case, the while-loop ends, and there is no 
		syslog error appended to the log, since read() did not return an error */  
		if ((numRead = read(client_fd, buf, BUF_SIZE)) > 0) {
			/* add debug info to see amount of bytes received from client */
			printf("%ld Bytes received from client.", numRead);

			/* print message received to STDOUT */
			printf("%s", buf);
		} // read()

		/* free resources */
		free(buf);

		if (numRead == -1) {
			errExit("read() failed.");
		}

		/* EOF - client closed socket */
		if (numRead == 0){
			puts("Received EOF from client!");
			break; /* break out of for-loop after EOF */
		}
	}//infinite for-loop
	
	/* kill  receiveMessages() child process, after this command, all child processes created for
	a particular client should be gone!! this fixes the bug present in pre-release v0.1.0-alpha */
	_exit(EXIT_SUCCESS);

}


int
main(int argc, char *argv[])
{
    int listen_fd, client_fd;	/* server listening socket and client socket */
	
	/* configure signal handling for SIGCHLD */
	if(configureSignalDisposition()==-1){
        errExit("[ERROR] Error: configureSignalDisposition()");
	}

	/* server listens on port, with a certain BACKLOG_QUEUE, and does not want to 
	receive information about the address of the client socket (NULL) */
    listen_fd = serverListen("50000", BACKLOG_QUEUE, NULL);
    if (listen_fd == -1) {
		/* The listening socket could not be created. */
        errExit("[ERROR] Could not create server listening socket");
		/* this error happens, if the listening PORT needs sudo rights to run, and
		the program is not run with sudo rights */
    }

	puts("[DEBUG] Server is listening on incomming connections.");

    for (;;) {
        client_fd = accept(listen_fd, NULL, NULL);  /* Wait for connection from client */
        if (client_fd == -1) {
            errExit("[ERROR] Failure in accept()");
       }
	
        /* Multi-process server back-end architecture:
		Handle each client request in a new child process */
        switch (fork()) {
		/* an error occured with fork() syscall, no children were created, this error is still handled 
		by the parent process */
        case -1:
            errExit("[ERROR] Error fork() call. Can't create child.");
            close(client_fd);         	/* Give up on this client */
            break;                      /* May be temporary; try next client */

		/* Child process (returns 0) */
        case 0:                       			
            puts("[DEBUG] Child process initialized (handling client connection)");
            close(listen_fd);           /* Unneeded copy of listening socket */
            receiveMessages(client_fd);	/* handleRequest() needs to have the client_fd as
										an input parameter, because it would otherwise not know
										to which and from which file descriptor to perform
										write and read calls */
			puts("[DEBUG] Child process terminated.");
            _exit(EXIT_SUCCESS);		/* child processes should generally only call _exit();
										for a more general discussion about the topic check
										25.4 'The Linux Programming Interface' */

	 	/* Parent: fork() actually returns the PID of the newly created child process */
        default:                       
            close(client_fd);           /* Unneeded copy of connected socket */
            break;                      /* Loop to accept next connection */
        } // end switch-case after fork()
    } // end for-loop accept() clients
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU AGPLv3 */
