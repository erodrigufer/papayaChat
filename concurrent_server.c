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
/* libraries needed to print the pid of a process, */
#include <sys/types.h>
#include <unistd.h>

/* This is how the getpid() function is declared,
this functions always work (see documentation man 2 getpid [getpid is a kernel system call
so it is found on (2)])
       pid_t getpid(void);
       pid_t getppid(void); */

#include "daemonCreation.h"		/* Create a daemon with one function call */
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */
#include "basics.h"
#include "configure_syslog.h"	/* handle configuration of syslog */

#define SERVICE "51000"          /* Name of TCP service */
#define BACKLOG_QUEUE 10		/* max number of clients in listening backlog queue */
#define BUF_SIZE 4096

/* static: function only used inside this file 
the signal handler receives as parameter an integer to differentiate 
the signal that triggered the signal handler 
In this case the grimReaper function is only triggered when a child process
is either killed or exits, so the parent process can perform a nonblocking wait
to prevent any children becoming zombie processes! */
/* TODO: check that all syscalls used in grimReaper() are async-safe syscalls */
static void             /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig)
{ 	/* SIGTERM is the default signal sent to a process when the 'kill' command is used 
	in the terminal (when no other signal is specified). Use this signal to kill the 
	parent/listening server! */
	if(sig==SIGTERM){
		syslog(LOG_DEBUG, "SIGTERM signal received. Killing process!");
		/* if exit is not specified then daemon does not terminate, and
		must be killed with a SIGKILL signal */
		exit(EXIT_SUCCESS);
	}
	if(sig==SIGCHLD){
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
	/* TODO: block all signal handling before the daemon is created */
    int listen_fd, client_fd;               /* server listening socket and client socket */
    struct sigaction sa_sigchild;					/* struc is necessary to define signals mask
											to be blocked during signal handler, needed 
											for syscall sigaction*/
	struct sigaction sa_term;				/* struc is necessary to define signals mask
											to be blocked during signal handler, needed 
											for syscall sigaction*/


	/* server should run as a daemon */
    if (daemonCreation(0) == -1)
        errExit("daemonCreation");			/* daemon creation failed, abort program
											core dump if EF_DUMPCORE env variable set */

	/* configure the syslog API,
	a daemon does not have a controlling terminal, so it should output all of its 
	error messages to a log */
	/* TODO: handle error of configureSyslog*/
	configure_syslog();

    /* Establish SIGCHLD handler to reap terminated child processes,
	if SIGCHLD is gathered with waitpid() or wait() by parent, then child
	process becomes a zombie and resources (PIDs) are not used efficiently.
	SIGCHLD is sent by the kernel to a parent process when one of its childern terminates
	(either by calling exit() or as a result of being killed by a signal).

	sa_mask is the signal set of signals that would be blocked during the
	invocation of the handler
	-> create an empyte signal set, no signal blocked during invocation of handler */
    sigemptyset(&sa_sigchild.sa_mask);				/*TODO: sigemptyset() should also be handled 
											with if statement in case there is an error == -1 */

	/* if a syscall is interrupted by the SIGCHLD, the kernel should
	restart the syscall after handling the signal,
	for that the SA_RESTART flag is used. Not all syscalls can be
	properly restarted by the kernel, check 21.5 of 'The Linux 
	Programming Interface' */
    sa_sigchild.sa_flags = SA_RESTART;
	/* grimReaper is the function handler for a SIGCHLD signal */
    sa_sigchild.sa_handler = grimReaper;
	/* the new disposition for SIGCHLD signal is the grimReaper function, the old
	signal disposition is not stored anywhere (NULL) */
    if (sigaction(SIGCHLD, &sa_sigchild, NULL) == -1) {
		/* the server runs as a daemon, so no errors can be output to stderr, since
		there is no controlling terminal. All errors are going to be logged into the 
		syslog using the syslog API */
        syslog(LOG_ERR, "Error: sigaction(SIGCHLD): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

	/* the new disposition for SIGTERM signal is the terminateParent function, the old
	signal disposition is not stored anywhere (NULL) */
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
		/* the server runs as a daemon, so no errors can be output to stderr, since
		there is no controlling terminal. All errors are going to be logged into the 
		syslog using the syslog API */
        syslog(LOG_ERR, "Error: sigaction(SIGTERM): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

	/* server listens on port 'SERVICE', with a certain BACKLOG_QUEUE, and does not want to 
	receive information about the address of the client socket (NULL) */
    listen_fd = serverListen(SERVICE, BACKLOG_QUEUE, NULL);
    if (listen_fd == -1) {
		/* The listening socket could not be created. */
        syslog(LOG_ERR, "Could not create server listening socket (%s)", strerror(errno));
		/* this error happens, if the listening PORT needs sudo rights to run, and
		the program is not run with sudo rights */
        exit(EXIT_FAILURE);
    }

	/* send message to syslog, server is listening */
	syslog(LOG_DEBUG, "Server is listening on incomming connections.");
    for (;;) {
        client_fd = accept(listen_fd, NULL, NULL);  /* Wait for connection from client */
        if (client_fd == -1) {
            syslog(LOG_ERR, "Failure in accept(): %s", strerror(errno));
            exit(EXIT_FAILURE);				/* TODO: if accept() fails, should it try again?
											and not exit inmediately ?? Check man page of accept()
											for all possible errors, one error is probably if the
											internet is down! */
        }

        /* Multi-process server back-end architecture:
		Handle each client request in a new child process */
        switch (fork()) {
        case -1:
            syslog(LOG_ERR, "Error fork() call. Can't create child (%s)", strerror(errno));
            close(client_fd);         /* Give up on this client */
            break;                      /* May be temporary; try next client */

        case 0:                       /* Child */
			/* write debug to syslog with child's PID */
            syslog(LOG_DEBUG, "Child process initialized (handling client connection)");
            close(listen_fd);                 /* Unneeded copy of listening socket */
            handleRequest(client_fd);	/* handleRequest() needs to have the client_fd as
										an input parameter, because it would otherwise not know
										to which and from which file descriptor to perform
										write and read calls */
            _exit(EXIT_SUCCESS);		/* child processes should generally only call _exit();
										for a more general discussion about the topic check
										25.4 'The Linux Programming Interface' */

        default:                        /* Parent */
            close(client_fd);                 /* Unneeded copy of connected socket */
            break;                      /* Loop to accept next connection */
        } // end switch-case after fork()
    } // end for-loop accept() clients
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU AGPLv3 */
