/* concurrent_tcp_server.c

Back-end server handling clients of papayaChat

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
#include "daemonCreation.h"		/* Create a daemon with one function call */
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */
#include "basics.h"
#include "configure_syslog.h"	/* handle configuration of syslog */
#include "file_locking.h"		/* handle file locking (avoid race conditions),
								required here to open chat file log */
#include "signalHandling.h"		/* signal handlers library */
#include "clientRequest.h"		/* what server does with client requests */
#include "configParser.h"	/* function to parse config files */

#include "CONFIG.h"				/* add config file to define TCP port, 
								termAsync binary pathname, BUF_SIZE, backlog queue */

/* signal handler for SIGTERM signal */
static void 
termHandler(int sig)
{
	/* the first argument is always per convention the filename being executed,
	see execv(2), always finish the strings array with 'NULL' */
	char *termargv[] = { PATHNAME_TERM_ASYNC_SAFE, NULL };
	/* if execv fails, there is no way of safely knowing about the error, since syslog
	is not an async-safe function that can be used inside a signal handler */
	execv(PATHNAME_TERM_ASYNC_SAFE, termargv);

	/* we should not get to this point, if it so happens, execv produced an error,
	in that case exit with failure, _exit() is async-safe, exit(3) is not, since it
	calls at_exit() */
	_exit(EXIT_FAILURE);

}

/* configure SIGTERM signal handler, if sigaction() or sigfillset() fail it returns -1 */
static int
configureTermHandler(void)
{
	struct sigaction sa_sigterm;			/* struc is necessary to define signals mask
											to be blocked during signal handler, needed 
											for syscall sigaction*/
	
	/* during the SIGTERM handler all other signals are blocked, since the 
	process should terminate immediately */
	if(sigfillset(&sa_sigterm.sa_mask)==-1)
		return -1;

	/* termHandler is the function handler for a SIGTERM signal */
    sa_sigterm.sa_handler = termHandler;
	
	/* the new disposition for SIGTERM signal is termHandler(), the old
	signal disposition is not stored anywhere (NULL) */
    if (sigaction(SIGTERM, &sa_sigterm, NULL) == -1) 
		return -1;	/* sigaction() failed */

	return 0;	/* error handling outside the function
				because a daemon can only log errors with syslog */ 
}

/* parse PORT */
static void
getConfigValues(char * port_parsed)
{
	
	const char * server_config_file = "/etc/papayachat/server.config";

	/* parse PORT in server's config file */
	if(parseConfigFile(server_config_file, "PORT", port_parsed)==-1){
		syslog(LOG_ERR,"parseConfigFile for port failed: %s",strerror(errno));
		exit(EXIT_FAILURE);
	}

}

/* parse KEY */
static void
getKey(char * key)
{

	const char * key_file = "/etc/papayachat/key";

	/* parse KEY in key file */
	if(parseConfigFile(key_file, "KEY", key)==-1){
		syslog(LOG_ERR,"parseConfigFile for key failed: %s",strerror(errno));
		exit(EXIT_FAILURE);
	}

}

/* a timeout during authentication is configured, if the client does not respond
during the auth time, a SIGALRM is triggered and the child process is killed by the signal 
if auth key is correct, returns 0, otherwise it fails with -1 */
static int 
authClient(int client_fd, char * key)
{

	/* setup timeout for authentication process */
	if(configureTimeout()==-1){
		syslog(LOG_ERR, "timeout configuration during auth failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}

	ssize_t numRead;
	char * buf = (char *) malloc(KEY_LENGTH);
	/* if malloc fails, it returns a NULL pointer */
	if(buf == NULL){
		syslog(LOG_ERR, "malloc failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* timeout after only 1 second */
	alarm(1);
	/* if the client closes its connection, the previous read() syscall will get an
	EOF, and it will return 0, in that case, the while-loop ends, and there is no 
	syslog error appended to the log, since read() did not return an error 
	this call reads exactly the length of a KEY from the client */  
	if ((numRead = read(client_fd, buf, KEY_LENGTH)) > 0) {
		/* add debug syslog to see amount of bytes received from client */
		//syslog(LOG_DEBUG, "Received auth key from client. %d Bytes ", numRead);
		alarm(0); /* auth token read, turn off timeout */
		syslog(LOG_DEBUG, "Key received from client...");
		/* compare key received with system key for validity */
		if(strncmp(buf,key,KEY_LENGTH)==0){
			syslog(LOG_DEBUG, "[OK] Key received is valid.");
			free(buf);
			return 0; /* Auth succeded */
		}
		else{
			syslog(LOG_DEBUG, "[FAIL] Key received is NOT valid.");
			free(buf);
			return -1; /* Auth failed */
		}

	
	} // read()
	
	if (numRead == -1) {
		syslog(LOG_ERR, "key auth read() failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* EOF - client closed socket */
	if (numRead == 0){
		syslog(LOG_DEBUG, "Received EOF from client during authentication!");
		_exit(EXIT_FAILURE);
	}

	/* free resources */
	free(buf);
	/* the program control flow should actually never arrive here, so if it does it is an error */
	_exit(EXIT_FAILURE);

}

int
main(int argc, char *argv[])
{
    int listen_fd, client_fd;               /* server listening socket and client socket */
	
	/* server should run as a daemon, 
	DAEMON_FLAH_NO_CHDIR -> daemon should initially stay in the same 
	working directory to have access to other executables required */
    if (daemonCreation(DAEMON_FLAG_NO_CHDIR) == -1)
        errExit("daemonCreation");			/* daemon creation failed, abort program
											core dump if EF_DUMPCORE env variable set */

	/* configure the syslog API,
	a daemon does not have a controlling terminal, so it should output all of its 
	error messages to a log */
	configure_syslog("papayaChat(parent)");

	/* configure signal handling for SIGCHLD */
	if(configureSignalDisposition()==-1){
		/* the server runs as a daemon, so no errors can be output to stderr, since
		there is no controlling terminal. All errors are going to be logged into the 
		syslog using the syslog API */
        syslog(LOG_ERR, "Error: configureSignalDisposition(): %s", strerror(errno));
        exit(EXIT_FAILURE);
	}

	/* configure signal handling for SIGTERM */
    if (configureTermHandler() == -1) {
        syslog(LOG_ERR, "Error: sigaction(SIGTERM) or sigfillset(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

	/* open (or create) the central chat log file */
	int chatlog_fd = openChatLogFile();
	if(chatlog_fd == -1){
		syslog(LOG_ERR, "Error: open (create) chat log file: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* allocate memory to parse port */
	char * port_parsed = (char *) malloc(MAX_LINE_LENGTH+10);
	if(port_parsed==NULL){
		syslog(LOG_ERR, "malloc port_parsed failed: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* parse PORT from config file */
	getConfigValues(port_parsed);

	/* open file with authentication key and parse key from file */
	char * key = (char *) malloc(KEY_LENGTH);
	if(key==NULL){
		syslog(LOG_ERR, "malloc key failed: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	getKey(key); /* get auth key */

	/* server listens on port, with a certain BACKLOG_QUEUE, and does not want to 
	receive information about the address of the client socket (NULL) */
    listen_fd = serverListen(port_parsed, BACKLOG_QUEUE, NULL);
	free(port_parsed);
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
		/* an error occured with fork() syscall, no children were created, this error is still handled 
		by the parent process */
        case -1:
            syslog(LOG_ERR, "Error fork() call. Can't create child (%s)", strerror(errno));
            close(client_fd);         	/* Give up on this client */
            break;                      /* May be temporary; try next client */

		/* Child process (returns 0) */
        case 0:                       			
		/* write debug to syslog with child's PID, new configuration of syslog */
			configure_syslog("papayaChat(child)");
            syslog(LOG_DEBUG, "Child process initialized (handling client connection)");
			/* Authenticate client with key */
			if(authClient(client_fd,key)==-1){
			/* if the client takes more than 1 second to send the key, then the function authClient() 
			times out, and the process receives a SIGALRM signal which terminates this child process
			IMPORTANT: No syslog has been implemented to write down that authClient has timed out */
				close(client_fd);
				syslog(LOG_INFO,"Auth failed. Client dropped! Terminating child process...");
				_exit(EXIT_SUCCESS); /* authentication failed, terminate child process, the exit is 
				successful, since not authenticating a client is a valid operation */
			}
            close(listen_fd);           /* Unneeded copy of listening socket */
			free(key);					/* key not needed on child process anymore */
            handleRequest(client_fd, chatlog_fd);	/* handleRequest() needs to have the client_fd as
										an input parameter, because it would otherwise not know
										to which and from which file descriptor to perform
										write and read calls */
			syslog(LOG_INFO, "Child process terminated.");
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
