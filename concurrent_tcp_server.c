/* concurrent_tcp_server.c

   An implementation of the TCP "echo" service.

   NOTE: this program must be run under a root login, in order to allow the
   "echo" port (7) to be bound. Alternatively, for test purposes, you can
   replace the SERVICE name below with a suitable unreserved port number
   (e.g., "51000"), and make a corresponding change in the client.

*/
#include <signal.h>				/* check 'man 2 sigaction' signal.h is needed
								to change the disposition of signals with
								the sigaction() syscall */	
#include <syslog.h>				/* server runs as a daemon, logging handled
								through syslog API */
#include <sys/wait.h>
#include "daemonCreation.h"		/* Create a daemon with one function call */
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */
#include "basics.h"

#define SERVICE "echo"          /* Name of TCP service */
#define BUF_SIZE 4096

/* static: function only used inside this file 
the signal handler receives as parameter an integer to differentiate 
the signal that triggered the signal handler */
static void             /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig)
{
    int savedErrno;             /* Save 'errno' in case changed here */
    savedErrno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

/* Handle a client request: copy socket input back to socket,
good coding practices, function is static, because it should only
be available in this local script */
static void
handleRequest(int client_fd)
{
    char buf[BUF_SIZE];
    ssize_t numRead;

    while ((numRead = read(client_fd, buf, BUF_SIZE)) > 0) {
        if (write(client_fd, buf, numRead) != numRead) {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        syslog(LOG_ERR, "Error from read(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char *argv[])
{
    int listen_fd, client_fd;               /* server listening socket and client socker */
    struct sigaction sa;					

	/* server should run as a daemon */
    if (daemonCreation(0) == -1)
        errExit("daemonCreation");			/* daemon creation failed, abort program
											core dump if EF_DUMPCORE env variable set */

    /* Establish SIGCHLD handler to reap terminated child processes,
	if SIGCHLD is not properly handled by the parent process, then 
	zombie processes could happen 

	SIGCHLD is sent by the kernel to a parent process when one of its childern terminates
	(either by calling exit() or as a result of being killed by a signal).

	sa_mask is the signal set of signals that would be blocked during the
	invocation of the handler
	-> create an empyte signal set, no signal blocked during invocation of handler */
    sigemptyset(&sa.sa_mask);				/*TODO: sigemptyset() should also be handled 
											with if statement in case there is an error == -1 */

	/* if a syscall is interrupted by the SIGCHLD, the kernel should
	restart the syscall after handling the signal,
	for that the SA_RESTART flag is used. Not all syscalls can be
	properly restarted by the kernel, check 21.5 of 'The Linux 
	Programming Interface' */
    sa.sa_flags = SA_RESTART;
	/* grimReaper is the function handler for a SIGCHLD signal */
    sa.sa_handler = grimReaper;
	/* the new disposition for SIGCHLD signal is the grimReaper function, the old
	signal disposition is not stored anywhere (NULL) */
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(SIGCHLD): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    listen_fd = serverListen(SERVICE, 10, NULL);
    if (listen_fd == -1) {
		/* The listening socket could not be created. */
        syslog(LOG_ERR, "Could not create server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;) {
        client_fd = accept(listen_fd, NULL, NULL);  /* Wait for connection */
        if (client_fd == -1) {
            syslog(LOG_ERR, "Failure in accept(): %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* Handle each client request in a new child process */

        switch (fork()) {
        case -1:
            syslog(LOG_ERR, "Can't create child (%s)", strerror(errno));
            close(client_fd);                 /* Give up on this client */
            break;                      /* May be temporary; try next client */

        case 0:                         /* Child */
            close(listen_fd);                 /* Unneeded copy of listening socket */
            handleRequest(client_fd);
            _exit(EXIT_SUCCESS);

        default:                        /* Parent */
            close(client_fd);                 /* Unneeded copy of connected socket */
            break;                      /* Loop to accept next connection */
        }
    }
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU AGPLv3 */
